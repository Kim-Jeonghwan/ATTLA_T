// UdpEthProtocol.cs
// Nexcom Co., Ltd.
// Description : ATTLA-T 이더넷(UDP) 프로토콜 구현 (Ethernet_Specification.md 규격 준수)

using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace ATTLA_T_PC
{
    public class UdpEthProtocol : IProtocol
    {
        // ── 네트워크 설정 ─────────────────────────────────────
        private const string DspIpAddress   = "192.168.200.10";
        private const string LocalIpAddress = "192.168.200.100";
        private const int    DspRxPort      = 5001;  
        private const int    PcRxPort       = 5003; 

        // ── 규격서 프로토콜 상수 ──────────────────────────────
        private const byte SrcIdPc          = 0x01;
        private const byte DstIdDsp         = 0x02;
        private const byte MsgCodeBoot      = 0x10;
        private const byte MsgCodeStatusReq = 0x11;
        private const byte MsgCodeAck       = 0xFF;
        private const byte MsgCodePbitReq     = 0x12;
        private const byte MsgCodePbitRep     = 0x13;
        private const byte MsgCodeIbitReq     = 0x14;
        private const byte MsgCodeIbitRep     = 0x15;
        private const byte MsgCodeCbitSet     = 0x16;
        private const byte MsgCodeCbitRep     = 0x17;
        private const byte MsgCodeIbitDone    = 0x19;
        private const byte MsgCodeIbitResReq  = 0x1A;
        private const byte MsgCodeCbitStop    = 0x1B;
        private const byte PriorityEmerg    = 0x01;
        private const byte PriorityNormal   = 0x02;

        // ── 내부 상태 ─────────────────────────────────────────
        private UdpClient?      _udpClient;
        private IPEndPoint?     _dspEndPoint;
        private IPEndPoint?     _localEndPoint;
        private Thread?         _rxThread;
        private volatile bool   _keepReceiving;
        private volatile bool   _commError;
        private readonly object _sendLock = new object();

        // ── IProtocol 이벤트 ──────────────────────────────────
        public event Action<StatusMessageData>? OnStatusReceived;
        public event Action<string>?            OnCommError;
        public event Action?                    OnPortClosed;
        public event Action<byte[]>?            OnRawTx;
        public event Action<byte[]>?            OnRawRx;
        public event Action<string, uint>?      OnBitResultReceived;
        public event Action?                    OnIbitDoneReceived;
        public event Action<byte, bool>?        OnAckReceived;
        public bool IsConnected => _udpClient != null;

        public void Connect(string portName, int baudRate)
        {
            if (IsConnected) Disconnect();

            _commError        = false;
            _localEndPoint    = new IPEndPoint(IPAddress.Parse(LocalIpAddress), PcRxPort);
            _dspEndPoint      = new IPEndPoint(IPAddress.Parse(DspIpAddress), DspRxPort);

            _udpClient = new UdpClient();
            _udpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            _udpClient.Client.Bind(_localEndPoint);

            _keepReceiving = true;
            _rxThread = new Thread(ReceiveWorker) { IsBackground = true };
            _rxThread.Start();
        }

        public void Disconnect()
        {
            _keepReceiving = false;
            try { _udpClient?.Close(); } catch { }
            _rxThread?.Join(500);
            _udpClient = null;
            OnPortClosed?.Invoke();
        }

        public void ReInit()
        {
            Disconnect();
            Connect("", 0);
        }

        public void SendControlMessage(ControlMessageData ctrlDto)
        {
            // 사용하지 않음
        }

        public void SendEthCommand(byte cmdCode, byte[] payload)
        {
            if (_udpClient == null) return;
            lock (_sendLock)
            {
                int pLen = (payload != null) ? payload.Length : 0;
                byte[] packet = new byte[14 + pLen];
                uint ts = (uint)(Environment.TickCount & 0x7FFFFFFF);

                packet[0] = (byte)(ts & 0xFF);
                packet[1] = (byte)((ts >> 8)  & 0xFF);
                packet[2] = (byte)((ts >> 16) & 0xFF);
                packet[3] = (byte)((ts >> 24) & 0xFF);
                packet[4] = SrcIdPc;
                packet[5] = DstIdDsp;
                packet[6] = cmdCode;
                packet[7] = 0x01; // ReqAck
                packet[8] = 0x02; // Priority Normal
                packet[9] = 1;
                packet[10] = (byte)(pLen & 0xFF);
                packet[11] = (byte)((pLen >> 8) & 0xFF);
                
                if (payload != null)
                {
                    Array.Copy(payload, 0, packet, 12, pLen);
                }

                ushort chk = CalcChecksum(packet, 12 + pLen);
                packet[12 + pLen] = (byte)(chk & 0xFF);
                packet[13 + pLen] = (byte)((chk >> 8) & 0xFF);

                SendPayload(packet);
            }
        }

        private void ReceiveWorker()
        {
            IPEndPoint remoteEp = new IPEndPoint(IPAddress.Any, 0);
            while (_keepReceiving)
            {
                try
                {
                    byte[]? data = _udpClient?.Receive(ref remoteEp);
                    if (data == null || data.Length == 0) continue;

                    OnRawRx?.Invoke(data);
                    ProcessReceivedUdpPayload(data);
                }
                catch (SocketException) { if (_keepReceiving) OnCommError?.Invoke("UDP 수신 오류"); break; }
                catch (ObjectDisposedException) { break; }
            }
        }

        private void ProcessReceivedUdpPayload(byte[] data)
        {
            if (data.Length < 14) return;

            byte srcId = data[4];
            byte code  = data[6];

            if (srcId == DstIdDsp && code == MsgCodeBoot)
            {
                if (!VerifyChecksum(data, 14)) return;
                
                SendAck(MsgCodeBoot, 0x0000);
                
                var msg = new StatusMessageData { IncNumber = 0, Status = 0x10, DspTemp = 0, IsCommError = false };
                OnStatusReceived?.Invoke(msg);
            }
            else if (srcId == DstIdDsp && code == MsgCodeStatusReq)
            {
                if (!VerifyChecksum(data, 15)) return;
                
                var msg = new StatusMessageData { IncNumber = 1, Status = 0, DspTemp = 0, IsCommError = false };
                OnStatusReceived?.Invoke(msg);
            }

            else if (srcId == DstIdDsp && code == MsgCodePbitRep)
            {
                if (!VerifyChecksum(data, 18)) return;
                uint bitResult = BitConverter.ToUInt32(data, 12);
                OnBitResultReceived?.Invoke("PBIT", bitResult);
            }
            else if (srcId == DstIdDsp && code == MsgCodeCbitRep)
            {
                if (!VerifyChecksum(data, 18)) return;
                uint bitResult = BitConverter.ToUInt32(data, 12);
                OnBitResultReceived?.Invoke("CBIT", bitResult);
            }
            else if (srcId == DstIdDsp && code == MsgCodeIbitRep)
            {
                if (!VerifyChecksum(data, 18)) return;
                uint bitResult = BitConverter.ToUInt32(data, 12);
                OnBitResultReceived?.Invoke("IBIT", bitResult);
            }
            else if (srcId == DstIdDsp && code == MsgCodeIbitDone)
            {
                if (!VerifyChecksum(data, 14)) return;
                SendAck(MsgCodeIbitDone, 0x0000);
                OnIbitDoneReceived?.Invoke();
            }
            else if (srcId == DstIdDsp && code == MsgCodeAck)
            {
                if (!VerifyChecksum(data, 18)) return;
                byte ackResult = data[7]; // 0x10=ACK, 0x11=NACK
                byte targetCode = data[12];
                OnAckReceived?.Invoke(targetCode, ackResult == 0x10);
            }
        }

        private void SendAck(byte targetCode, ushort ackResult)
        {
            if (_udpClient == null) return;
            lock (_sendLock)
            {
                byte[] payload = new byte[18];
                uint ts = (uint)(Environment.TickCount & 0x7FFFFFFF);

                payload[0] = (byte)(ts & 0xFF);
                payload[1] = (byte)((ts >> 8)  & 0xFF);
                payload[2] = (byte)((ts >> 16) & 0xFF);
                payload[3] = (byte)((ts >> 24) & 0xFF);
                payload[4] = SrcIdPc;
                payload[5] = DstIdDsp;
                payload[6] = MsgCodeAck;
                payload[7] = 0xFF; // ReqAck none
                payload[8] = PriorityNormal;
                payload[9] = 1;
                payload[10] = 0x04;
                payload[11] = 0x00;
                payload[12] = targetCode;
                payload[13] = 0x00;
                payload[14] = (byte)(ackResult & 0xFF);
                payload[15] = (byte)((ackResult >> 8) & 0xFF);

                ushort chk = CalcChecksum(payload, 16);
                payload[16] = (byte)(chk & 0xFF);
                payload[17] = (byte)((chk >> 8) & 0xFF);

                SendPayload(payload);
            }
        }

        private static ushort CalcChecksum(byte[] buf, int length)
        {
            uint sum = 0;
            for (int i = 0; i < length; i++) sum += buf[i];
            return (ushort)(sum & 0xFFFF);
        }

        private static bool VerifyChecksum(byte[] data, int totalLen)
        {
            if (data.Length < totalLen) return false;
            int chkOffset = totalLen - 2;
            ushort recvChk = (ushort)(data[chkOffset] | (data[chkOffset + 1] << 8));
            ushort calcChk = CalcChecksum(data, chkOffset);
            return recvChk == calcChk;
        }

        private void SendPayload(byte[] payload)
        {
            try
            {
                _udpClient?.Send(payload, payload.Length, _dspEndPoint);
                OnRawTx?.Invoke(payload);
            }
            catch (Exception ex) { OnCommError?.Invoke($"UDP 전송 오류: {ex.Message}"); }
        }
    }
}
