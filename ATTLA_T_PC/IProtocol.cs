using System;

namespace ATTLA_T_PC
{
    public class StatusMessageData
    {
        public byte IncNumber { get; set; }
        public byte Status { get; set; }
        public double DspTemp { get; set; }
        public bool IsCommError { get; set; }
    }

    public class ControlMessageData
    {
        public byte ManualSeqNum { get; set; }
    }

    public interface IProtocol
    {
        bool IsConnected { get; }
        event Action<StatusMessageData> OnStatusReceived;
        event Action<string> OnCommError;
        event Action OnPortClosed;
        event Action<byte[]> OnRawTx;
        event Action<byte[]> OnRawRx;

        event Action<string, uint> OnBitResultReceived;
        event Action OnIbitDoneReceived;
        event Action<byte, bool> OnAckReceived;

        void Connect(string portName, int baudRate);
        void Disconnect();
        void ReInit();
        void SendControlMessage(ControlMessageData ctrlDto);
        void SendEthCommand(byte cmdCode, byte[] payload);
    }
}
