/*
 * File: MainForm.cs
 * Created: 2026-06-01 (Modified by Antigravity)
 * Description: ATTLA_T PC Monitoring Dashboard MainForm (Refactored for Layout and BIT requirements)
 * Last Updated: 2026. 06. 24.
 */

using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO.Ports;
using System.Windows.Forms;
using System.Management;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Linq;

namespace ATTLA_T_PC
{
    public class MainForm : Form
    {
        private IProtocol _protocol;
        private System.Windows.Forms.Timer _timer;
        private System.Windows.Forms.Timer _statusTimer; // 100ms 통신상태 확인용 타이머

        private Color colorBg = Color.FromArgb(30, 30, 30);
        private Color colorPanelBg = Color.FromArgb(45, 45, 48);
        private Color colorText = Color.FromArgb(220, 220, 220);
        private Color colorAccent = Color.FromArgb(0, 122, 204);
        private Color colorError = Color.FromArgb(200, 50, 50);

        private DateTime _lastRxTime = DateTime.MinValue;

        private ControlMessageData _ctrlDto = new ControlMessageData();

        private ComboBox cmbPorts;
        private ComboBox cmbBauds;
        private Button btnConnect;
        private Button btnDisconnect;
        private Button btnInit;
        private Button btnRefresh;
        private Label lblPortConnected;
        private Label lblCommReceiving;

        // Status & Control UI
        private Label lblBoardTemp;
        
        private Label lblBootStatus;
        private Label lblStatusInfo;

        private Label[] lblPbitFaults;
        private Label[] lblCbitFaults;
        private Label[] lblIbitFaults;

        private int _cbitCount = 0;
        private int _ibitCount = 0;
        private Label lblCbitCount;
        private Label lblIbitCount;

        private Label lblCbitSetResult;
        private Label lblIbitReqStatus;

        private TextBox txtCbitPeriod;
        private TextBox txtIbitDuration;

        // Log
        private LogForm _logForm;
        private Label lblLogRxInfo;
        private Label lblLogTxInfo;

        // 프로토콜 선택 UI
        private RadioButton rdoSci;
        private RadioButton rdoUdp;
        private Label lblCommStatus;  // 통신 두절 표시


        public MainForm()
        {
            this.Text = "ATTLA_T Monitoring & Dashboard";
            this.Size = new Size(2200, 1400); // Expanded Size
            this.MinimumSize = new Size(2200, 1400);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.BackColor = colorBg;
            this.ForeColor = colorText;
            this.Font = new Font("맑은 고딕", 9F);

            _logForm = new LogForm();

            _protocol = new SciPcProtocol();
            SetupProtocolEvents();
            SetupMenu();

            _timer = new System.Windows.Forms.Timer { Interval = 100 }; // 100ms UI Update
            _timer.Tick += Timer_Tick;

            _statusTimer = new System.Windows.Forms.Timer { Interval = 100 };
            _statusTimer.Tick += StatusTimer_Tick;


            BuildUI();
        }


        private void StatusTimer_Tick(object sender, EventArgs e)
        {
            if (_protocol is UdpEthProtocol && _protocol.IsConnected)
            {
                _protocol.SendEthCommand(0x11, null); // ETH_CODE_STATUS_REQ
            }
        }

        private void BuildUI()
        {
            TableLayoutPanel mainLayout = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                RowCount = 4,
                ColumnCount = 1,
                Padding = new Padding(15, 40, 15, 15)
            };
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 220)); // Comm Panel
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 450)); // Status Panel (Height Expanded for Spacing)
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 250)); // Control Panel (Height Expanded)
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));  // Log Panel

            // 1. Top Bar - Communication Panel
            Panel pnlComm = CreateStyledPanel("COMMUNICATION (PORT / BAUD RATE)");
            pnlComm.Dock = DockStyle.Fill;
            pnlComm.Margin = new Padding(5);

            int commY = 50;
            Label lblPort = new Label { Text = "COM Port", Location = new Point(20, commY), AutoSize = true, Font = new Font("Segoe UI", 11, FontStyle.Bold) };
            cmbPorts = new ComboBox
            {
                Location = new Point(160, commY - 2),
                Size = new Size(350, 35),
                BackColor = Color.FromArgb(45, 45, 48),
                ForeColor = Color.FromArgb(0, 255, 200),
                DropDownStyle = ComboBoxStyle.DropDownList,
                Font = new Font("맑은 고딕", 11)
            };
            UpdatePortsList();

            btnRefresh = CreateBorderedButton("새로고침", 530, commY - 5, 120, 40); 
            btnRefresh.Click += (s, e) => UpdatePortsList();

            lblPortConnected = new Label { Text = "● 포트 연결됨", Location = new Point(680, commY), AutoSize = true, ForeColor = Color.Gray, Font = new Font("맑은 고딕", 11, FontStyle.Bold) }; 
            lblCommReceiving = new Label { Text = "● 통신 수신중", Location = new Point(880, commY), AutoSize = true, ForeColor = Color.Gray, Font = new Font("맑은 고딕", 11, FontStyle.Bold) }; 

            Label lblBaud = new Label { Text = "Baud Rate", Location = new Point(20, commY + 50), AutoSize = true, Font = new Font("Segoe UI", 11, FontStyle.Bold) };
            cmbBauds = new ComboBox
            {
                Location = new Point(160, commY + 48),
                Size = new Size(350, 35), 
                BackColor = Color.FromArgb(45, 45, 48),
                ForeColor = Color.FromArgb(0, 255, 200),
                DropDownStyle = ComboBoxStyle.DropDownList,
                Font = new Font("맑은 고딕", 11)
            };
            cmbBauds.Items.AddRange(new object[] { "9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600" });
            cmbBauds.SelectedItem = "115200";

            btnConnect = CreateBorderedButton("연결", 530, commY + 45, 100, 40); 
            btnConnect.Click += (s, e) => Connect();

            btnDisconnect = CreateBorderedButton("해제", 640, commY + 45, 100, 40); 
            btnDisconnect.Click += (s, e) => _protocol.Disconnect();

            btnInit = CreateBorderedButton("초기화", 750, commY + 45, 100, 40); 
            btnInit.Click += (s, e) => _protocol.ReInit();

            /* 프로토콜 선택: SCI / UDP */
            rdoSci = new RadioButton
            {
                Text = "SCI (기본)",
                Location = new Point(20, commY + 105),
                AutoSize = true,
                Checked = true,
                Font = new Font("맑은 고딕", 11, FontStyle.Bold),
                ForeColor = Color.Cyan
            };
            rdoUdp = new RadioButton
            {
                Text = "UDP (Ethernet)",
                Location = new Point(200, commY + 105),
                AutoSize = true,
                Font = new Font("맑은 고딕", 11, FontStyle.Bold),
                ForeColor = Color.Orange
            };
            rdoUdp.CheckedChanged += (s, e) =>
            {
                /* UDP 선택 시 COM Port/Baud 콤보박스 비활성화 */
                cmbPorts.Enabled = rdoSci.Checked;
                cmbBauds.Enabled = rdoSci.Checked;
            };

            lblCommStatus = new Label
            {
                Text = "",
                Location = new Point(680, commY + 50),
                Size = new Size(250, 30),
                Font = new Font("맑은 고딕", 11, FontStyle.Bold),
                ForeColor = Color.Red
            };

            pnlComm.Controls.AddRange(new Control[] {
                lblPort, lblBaud, cmbPorts, cmbBauds, btnRefresh,
                btnConnect, btnDisconnect, btnInit,
                lblPortConnected, lblCommReceiving,
                rdoSci, rdoUdp, lblCommStatus
            });
            mainLayout.Controls.Add(pnlComm, 0, 0);


            // 2. Status Panel - BIT Status & Board Temperature
            Panel pnlStatus = CreateStyledPanel("MCU STATUS MONITOR (수신 데이터)");
            pnlStatus.Dock = DockStyle.Fill;
            pnlStatus.Margin = new Padding(10);

            Label lblTempTitle = new Label { Text = "DSP 상태:", Location = new Point(40, 50), AutoSize = true, Font = new Font("맑은 고딕", 16, FontStyle.Bold), ForeColor = Color.Orange };
            lblBoardTemp = new Label { Text = "대기 중", Location = new Point(200, 48), AutoSize = true, Font = new Font("Consolas", 18, FontStyle.Bold), ForeColor = Color.White };

            lblBootStatus = new Label { Text = "통신망 가입 상태: 대기 중...", Location = new Point(730, 50), AutoSize = true, Font = new Font("맑은 고딕", 16, FontStyle.Bold), ForeColor = Color.Yellow };
            lblStatusInfo = new Label { Text = "통신 상태 확인: 대기 중", Location = new Point(1470, 50), AutoSize = true, Font = new Font("맑은 고딕", 16, FontStyle.Bold), ForeColor = Color.Gray };
            pnlStatus.Controls.AddRange(new Control[] { lblTempTitle, lblBoardTemp, lblBootStatus, lblStatusInfo });

            // Fault LEDs Setup
            string[] faultNames = { "Mot OVC", "Brk OVC", "Bd OVT", "28V OVV", "Brk24V OVV", "DRV Fault", "Stall", "OverSpeed", "Enc Error", "Enc Warn" };
            
            // Layout adjusted for spacing
            GroupBox grpPbit = new GroupBox { Text = "PBIT 결과", Location = new Point(40, 120), Size = new Size(580, 280), ForeColor = Color.Cyan, Font = new Font("맑은 고딕", 14, FontStyle.Bold) };
            GroupBox grpCbit = new GroupBox { Text = "CBIT 결과", Location = new Point(650, 120), Size = new Size(580, 280), ForeColor = Color.MediumSpringGreen, Font = new Font("맑은 고딕", 14, FontStyle.Bold) };
            GroupBox grpIbit = new GroupBox { Text = "IBIT 결과", Location = new Point(1260, 120), Size = new Size(580, 280), ForeColor = Color.DodgerBlue, Font = new Font("맑은 고딕", 14, FontStyle.Bold) };

            lblPbitFaults = new Label[faultNames.Length];
            lblCbitFaults = new Label[faultNames.Length];
            lblIbitFaults = new Label[faultNames.Length];

            lblCbitCount = new Label { Text = "누적 수행 횟수: 0회", Location = new Point(350, 30), AutoSize = true, ForeColor = Color.White, Font = new Font("맑은 고딕", 12) };
            lblIbitCount = new Label { Text = "누적 수행 횟수: 0회", Location = new Point(350, 30), AutoSize = true, ForeColor = Color.White, Font = new Font("맑은 고딕", 12) };
            grpCbit.Controls.Add(lblCbitCount);
            grpIbit.Controls.Add(lblIbitCount);

            for (int i = 0; i < faultNames.Length; i++)
            {
                // Spacing out items more (3 columns, rows spaced out)
                Point loc = new Point(30 + (i % 3) * 180, 60 + (i / 3) * 55);
                
                lblPbitFaults[i] = new Label { Text = "● " + faultNames[i], Location = loc, AutoSize = true, Font = new Font("맑은 고딕", 12, FontStyle.Bold), ForeColor = Color.Gray };
                lblCbitFaults[i] = new Label { Text = "● " + faultNames[i], Location = loc, AutoSize = true, Font = new Font("맑은 고딕", 12, FontStyle.Bold), ForeColor = Color.Gray };
                lblIbitFaults[i] = new Label { Text = "● " + faultNames[i], Location = loc, AutoSize = true, Font = new Font("맑은 고딕", 12, FontStyle.Bold), ForeColor = Color.Gray };

                grpPbit.Controls.Add(lblPbitFaults[i]);
                grpCbit.Controls.Add(lblCbitFaults[i]);
                grpIbit.Controls.Add(lblIbitFaults[i]);
            }

            pnlStatus.Controls.Add(grpPbit);
            pnlStatus.Controls.Add(grpCbit);
            pnlStatus.Controls.Add(grpIbit);
            mainLayout.Controls.Add(pnlStatus, 0, 1);


            // 3. Control Panel - CBIT & IBIT Control
            Panel pnlCtrl = CreateStyledPanel("MCU CONTROL (송신 데이터)");
            pnlCtrl.Dock = DockStyle.Fill;
            pnlCtrl.Margin = new Padding(10);

            // CBIT Controls
            GroupBox grpCbitCtrl = new GroupBox { Text = "CBIT 제어", Location = new Point(40, 50), Size = new Size(800, 150), ForeColor = Color.MediumSpringGreen, Font = new Font("맑은 고딕", 12, FontStyle.Bold) };
            
            Label lblCbitInput = new Label { Text = "주기 (초):", Location = new Point(20, 50), AutoSize = true, Font = new Font("맑은 고딕", 14, FontStyle.Bold), ForeColor = Color.White };
            txtCbitPeriod = new TextBox { 
                Location = new Point(160, 48), 
                Width = 60, 
                BackColor = Color.FromArgb(60, 60, 60), 
                ForeColor = Color.White, 
                Text = "1", 
                BorderStyle = BorderStyle.FixedSingle, 
                Font = new Font("Consolas", 14) 
            };
            
            Button btnCbitSet = CreateBorderedButton("설정 적용", 240, 40, 120, 45);
            btnCbitSet.BackColor = Color.MediumSpringGreen;
            btnCbitSet.ForeColor = Color.Black;
            btnCbitSet.Click += (s, e) => 
            {
                if (!(_protocol is UdpEthProtocol))
                {
                    MessageBox.Show("CBIT 설정은 UDP(Ethernet) 모드에서만 가능합니다.", "통신 모드 확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                
                if (ushort.TryParse(txtCbitPeriod.Text, out ushort cbitPeriod))
                {
                    lblCbitSetResult.Text = "설정 전송 중...";
                    lblCbitSetResult.ForeColor = Color.Yellow;
                    byte[] payload = new byte[2];
                    payload[0] = (byte)(cbitPeriod & 0xFF);
                    payload[1] = (byte)((cbitPeriod >> 8) & 0xFF);
                    _protocol.SendEthCommand(0x16, payload); // ETH_CODE_CBIT_SET
                }
                else MessageBox.Show("올바른 주기 값을 입력하세요.", "입력 오류");
            };

            lblCbitSetResult = new Label { Text = "대기 중", Location = new Point(380, 52), AutoSize = true, Font = new Font("맑은 고딕", 12, FontStyle.Bold), ForeColor = Color.Gray };

            Button btnCbitReq = CreateBorderedButton("CBIT 시작", 570, 40, 120, 45);
            btnCbitReq.BackColor = Color.CornflowerBlue;
            btnCbitReq.Click += (s, e) => {
                if (!(_protocol is UdpEthProtocol)) return;
                _protocol.SendEthCommand(0x1C, null); // 0x1C : ETH_CODE_CBIT_REQ (시작)
            };
            
            Button btnCbitStop = CreateBorderedButton("CBIT 중지", 700, 40, 120, 45);
            btnCbitStop.BackColor = Color.OrangeRed;
            btnCbitStop.Click += (s, e) => {
                if (!(_protocol is UdpEthProtocol)) return;
                _protocol.SendEthCommand(0x1B, null); // 0x1B : ETH_CODE_CBIT_STOP (중지)
            };

            grpCbitCtrl.Controls.AddRange(new Control[] { lblCbitInput, txtCbitPeriod, btnCbitSet, lblCbitSetResult, btnCbitReq, btnCbitStop });


            // IBIT Controls
            GroupBox grpIbitCtrl = new GroupBox { Text = "IBIT 제어", Location = new Point(880, 50), Size = new Size(950, 150), ForeColor = Color.DodgerBlue, Font = new Font("맑은 고딕", 12, FontStyle.Bold) };

            Label lblIbitInput = new Label { Text = "수행 시간 (초):", Location = new Point(20, 50), AutoSize = true, Font = new Font("맑은 고딕", 14, FontStyle.Bold), ForeColor = Color.White };
            txtIbitDuration = new TextBox { 
                Location = new Point(260, 48), 
                Width = 60, 
                BackColor = Color.FromArgb(60, 60, 60), 
                ForeColor = Color.White, 
                Text = "5", 
                BorderStyle = BorderStyle.FixedSingle, 
                Font = new Font("Consolas", 14) 
            };

            Button btnIbitReq = CreateBorderedButton("IBIT 수행 요청", 340, 40, 160, 45);
            btnIbitReq.BackColor = Color.DodgerBlue;
            btnIbitReq.Click += (s, e) => 
            {
                if (!(_protocol is UdpEthProtocol))
                {
                    MessageBox.Show("IBIT 요청은 UDP(Ethernet) 모드에서만 가능합니다.", "통신 모드 확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    return;
                }
                if (ushort.TryParse(txtIbitDuration.Text, out ushort duration))
                {
                    lblIbitReqStatus.Text = "요청 전송 중...";
                    lblIbitReqStatus.ForeColor = Color.Yellow;
                    byte[] payload = new byte[2];
                    payload[0] = (byte)(duration & 0xFF);
                    payload[1] = (byte)((duration >> 8) & 0xFF);
                    _protocol.SendEthCommand(0x14, payload); // ETH_CODE_IBIT_REQ
                }
            };
            
            lblIbitReqStatus = new Label { Text = "대기 중", Location = new Point(520, 52), AutoSize = true, Font = new Font("맑은 고딕", 12, FontStyle.Bold), ForeColor = Color.Gray };

            Button btnIbitResReq = CreateBorderedButton("IBIT 결과 갱신", 750, 40, 160, 45);
            btnIbitResReq.Click += (s, e) => _protocol.SendEthCommand(0x1A, null); // ETH_CODE_IBIT_RES_REQ

            grpIbitCtrl.Controls.AddRange(new Control[] { lblIbitInput, txtIbitDuration, btnIbitReq, lblIbitReqStatus, btnIbitResReq });


            pnlCtrl.Controls.Add(grpCbitCtrl);
            pnlCtrl.Controls.Add(grpIbitCtrl);
            mainLayout.Controls.Add(pnlCtrl, 0, 2);


            // 4. Real-Time Log Panel
            Panel pnlLog = CreateStyledPanel("REAL-TIME LOG MONITOR");
            pnlLog.Dock = DockStyle.Fill;
            pnlLog.Margin = new Padding(5);

            lblLogRxInfo = new Label { Location = new Point(30, 55), AutoSize = true, Font = new Font("Consolas", 11, FontStyle.Bold), ForeColor = Color.Lime };
            lblLogTxInfo = new Label { Location = new Point(30, 115), AutoSize = true, Font = new Font("Consolas", 11, FontStyle.Bold), ForeColor = Color.Yellow };

            Button btnLogDetail = CreateBorderedButton("로그 상세 보기\n(6K History)", 0, 45, 180, 90);
            btnLogDetail.Anchor = AnchorStyles.Right | AnchorStyles.Top;
            btnLogDetail.Click += (s, e) =>
            {
                if (_logForm.IsDisposed) _logForm = new LogForm();
                _logForm.Show();
                _logForm.BringToFront();
            };

            pnlLog.Controls.Add(lblLogRxInfo);
            pnlLog.Controls.Add(lblLogTxInfo);
            pnlLog.Controls.Add(btnLogDetail);

            pnlLog.Resize += (s, e) => { btnLogDetail.Location = new Point(pnlLog.Width - 220, 45); };

            mainLayout.Controls.Add(pnlLog, 0, 3);

            this.Controls.Add(mainLayout);
            UpdateConnectButtons();
        }

        private void SetupMenu()
        {
            MenuStrip menuStrip = new MenuStrip
            {
                BackColor = Color.FromArgb(45, 45, 48),
                ForeColor = Color.White,
                Font = new Font("Segoe UI", 10),
                RenderMode = ToolStripRenderMode.System
            };

            ToolStripMenuItem fileMenu = new ToolStripMenuItem("File (&F)");
            fileMenu.ForeColor = Color.White;
            fileMenu.DropDown.BackColor = Color.FromArgb(45, 45, 48);
            fileMenu.DropDown.ForeColor = Color.White;

            ToolStripMenuItem exitItem = new ToolStripMenuItem("Exit (&X)", null, (s, e) => this.Close());
            exitItem.ForeColor = Color.White;
            fileMenu.DropDownItems.Add(exitItem);

            ToolStripMenuItem helpMenu = new ToolStripMenuItem("Help (&H)");
            helpMenu.ForeColor = Color.White;
            helpMenu.DropDown.BackColor = Color.FromArgb(45, 45, 48);
            helpMenu.DropDown.ForeColor = Color.White;

            ToolStripMenuItem aboutItem = new ToolStripMenuItem("About (&A)", null, (s, e) => {
                MessageBox.Show("ATTLA_T Monitoring & Dashboard\n\n" +
                                "Version: 1.2 (Refactored BIT Logic)\n" +
                                "Date: 2026. 06. 24.\n" +
                                "Developer: Kim Jeonghwan (Nexcom)", 
                                "About Program", 
                                MessageBoxButtons.OK, 
                                MessageBoxIcon.Information);
            });
            aboutItem.ForeColor = Color.White;
            helpMenu.DropDownItems.Add(aboutItem);

            menuStrip.Items.Add(fileMenu);
            menuStrip.Items.Add(helpMenu);

            this.MainMenuStrip = menuStrip;
            this.Controls.Add(menuStrip);
        }



        private void UpdatePortsList()
        {
            string selected = cmbPorts.SelectedItem as string;
            cmbPorts.Items.Clear();

            try
            {
                using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%(COM%)'"))
                {
                    var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();
                    var list = new List<string>();
                    
                    foreach (var p in ports)
                    {
                        string caption = p["Caption"].ToString();
                        list.Add(caption);
                    }

                    list.Sort((a, b) =>
                    {
                        var matchA = Regex.Match(a, @"\(COM(\d+)\)");
                        var matchB = Regex.Match(b, @"\(COM(\d+)\)");
                        int aNum = matchA.Success ? int.Parse(matchA.Groups[1].Value) : 0;
                        int bNum = matchB.Success ? int.Parse(matchB.Groups[1].Value) : 0;
                        return aNum.CompareTo(bNum);
                    });

                    foreach (var item in list) cmbPorts.Items.Add(item);
                }
            }
            catch
            {
                cmbPorts.Items.AddRange(SerialPort.GetPortNames());
            }

            if (cmbPorts.Items.Count > 0)
            {
                if (selected != null && cmbPorts.Items.Contains(selected))
                    cmbPorts.SelectedItem = selected;
                else
                    cmbPorts.SelectedIndex = 0;
            }
        }

        private void Connect()
        {
            try
            {
                if (_protocol != null && _protocol.IsConnected) _protocol.Disconnect();

                /* 프로토콜 선택: SCI 또는 UDP */
                if (rdoUdp != null && rdoUdp.Checked)
                {
                    _protocol = new UdpEthProtocol();
                }
                else
                {
                    if (cmbPorts.SelectedItem == null || cmbBauds.SelectedItem == null) return;
                    _protocol = new SciPcProtocol();
                }

                SetupProtocolEvents();

                if (rdoUdp != null && rdoUdp.Checked)
                {
                    /* UDP: portName/baudRate 인자는 무시됨 (클래스 내부 고정 IP 사용) */
                    _protocol.Connect("UDP", 0);
                }
                else
                {
                    string rawSelection = cmbPorts.SelectedItem.ToString();
                    string portName = rawSelection;
                    var match = Regex.Match(rawSelection, @"\((COM\d+)\)");
                    if (match.Success) portName = match.Groups[1].Value;
                    _protocol.Connect(portName, int.Parse(cmbBauds.SelectedItem.ToString()));
                }

                UpdateConnectButtons();
                _timer.Start();
                _firstStatusReceived = false;
                _statusTimer.Stop(); // 망 가입(Boot Done) 전까지 정지

            }
            catch (Exception ex)
            {
                MessageBox.Show("Connection Failed: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void SetupProtocolEvents()
        {
            _protocol.OnStatusReceived += OnStatusReceived;
            _protocol.OnCommError += OnCommError;
            _protocol.OnPortClosed += () => { if (!IsDisposed) Invoke((Action)UpdateConnectButtons); };
            _protocol.OnRawTx += OnRawTxReceived;
            _protocol.OnRawRx += OnRawRxReceived;
            
            _protocol.OnBitResultReceived += OnBitResultReceived;
            _protocol.OnIbitDoneReceived += OnIbitDoneReceived;
            _protocol.OnAckReceived += OnAckReceived;
        }

        private void UpdateConnectButtons()
        {
            bool isConn = _protocol.IsConnected;
            btnConnect.Enabled = !isConn;
            btnDisconnect.Enabled = isConn;
            btnInit.Enabled = isConn;

            if (isConn)
            {
                lblPortConnected.ForeColor = Color.Lime;
            }
            else
            {
                lblPortConnected.ForeColor = Color.Gray;
                lblCommReceiving.ForeColor = Color.Gray;
                _timer.Stop();
                _statusTimer.Stop();

            }
        }


        private bool _firstStatusReceived = false;
        private bool _statusToggle = false;

        private void OnStatusReceived(StatusMessageData data)
        {
            BeginInvoke((Action)(() =>
            {
                _lastRxTime = DateTime.Now;
                lblCommReceiving.ForeColor = Color.Lime;

                if (data.IncNumber == 0) // Boot Done
                {
                    lblBootStatus.Text = "통신망 가입 완료 (Boot Done 수신 및 ACK 전송)";
                    lblBootStatus.ForeColor = Color.Lime;
                    if (rdoUdp.Checked)
                    {
                        System.Threading.Thread.Sleep(100); 
                        if (_logForm != null && !_logForm.IsDisposed)
                            _logForm.AddLog("[BOOT] 망 가입 성공 (100ms 통신 상태 확인 시작)");
                        _statusTimer.Start();
                    }
                }
                else if (data.IncNumber == 1) // Status Response
                {
                    _statusToggle = !_statusToggle;
                    lblStatusInfo.Text = _statusToggle ? "통신 상태 확인: 정상 연결 중 ■" : "통신 상태 확인: 정상 연결 중 □";
                    lblStatusInfo.ForeColor = Color.Lime;
                    
                    if (!_firstStatusReceived && rdoUdp.Checked)
                    {
                        _firstStatusReceived = true;
                        if (_logForm != null && !_logForm.IsDisposed)
                            _logForm.AddLog("[PBIT] 첫 통신 성공 시 PBIT 자동 요청 발송");
                        _protocol.SendEthCommand(0x12, null); // ETH_CODE_PBIT_REQ
                    }
                }

                /* 통신 정상 수신 시 통신 두절 표시 해제 */
                if (lblCommStatus != null)
                    lblCommStatus.Text = "";
            }));
        }

        private void OnAckReceived(byte targetCode, bool isAck)
        {
            BeginInvoke((Action)(() =>
            {
                if (targetCode == 0x16) // CBIT_SET
                {
                    lblCbitSetResult.Text = isAck ? "주기 설정 완료" : "주기 설정 거부됨";
                    lblCbitSetResult.ForeColor = isAck ? Color.Lime : Color.Red;
                }
                else if (targetCode == 0x1B) // CBIT_STOP
                {
                    lblCbitSetResult.Text = isAck ? "중지 완료" : "중지 거부됨";
                    lblCbitSetResult.ForeColor = isAck ? Color.Yellow : Color.Red;
                }
                else if (targetCode == 0x1C) // CBIT_REQ
                {
                    lblCbitSetResult.Text = isAck ? "수신 중" : "요청 거부됨";
                    lblCbitSetResult.ForeColor = isAck ? Color.Lime : Color.Red;
                }
                else if (targetCode == 0x14) // IBIT_REQ
                {
                    lblIbitReqStatus.Text = isAck ? "IBIT 수행 중..." : "IBIT 요청 거부됨";
                    lblIbitReqStatus.ForeColor = isAck ? Color.Orange : Color.Red;
                    if (isAck)
                    {
                        lblCbitSetResult.Text = "IBIT 중 일시정지";
                        lblCbitSetResult.ForeColor = Color.Orange;
                    }
                }
                else if (targetCode == 0x1A) // IBIT_RES_REQ
                {
                    lblIbitReqStatus.Text = isAck ? "결과 갱신 요청됨" : "결과 갱신 거부됨";
                    lblIbitReqStatus.ForeColor = isAck ? Color.Lime : Color.Red;
                }
            }));
        }

        private void OnBitResultReceived(string type, uint bitmask)
        {
            BeginInvoke((Action)(() =>
            {
                Label[] targetFaults = lblPbitFaults;
                
                if (type == "CBIT")
                {
                    targetFaults = lblCbitFaults;
                    _cbitCount++;
                    lblCbitCount.Text = $"누적 수행 횟수: {_cbitCount}회";
                    lblCbitSetResult.Text = "수신 중";
                    lblCbitSetResult.ForeColor = Color.Lime;
                }
                else if (type == "IBIT")
                {
                    targetFaults = lblIbitFaults;
                    _ibitCount++;
                    lblIbitCount.Text = $"누적 수행 횟수: {_ibitCount}회";
                    lblIbitReqStatus.Text = "결과 갱신 완료";
                    lblIbitReqStatus.ForeColor = Color.Lime;
                }

                int[] bitPositions = { 8, 9, 11, 12, 13, 16, 17, 18, 19, 20 };
                for (int i = 0; i < bitPositions.Length; i++)
                {
                    bool isSet = ((bitmask >> bitPositions[i]) & 1) == 1;
                    targetFaults[i].ForeColor = isSet ? Color.Red : Color.Lime;
                }
            }));
        }

        private void OnIbitDoneReceived()
        {
            BeginInvoke((Action)(() =>
            {
                lblIbitReqStatus.Text = "IBIT 완료(결과 대기)";
                lblIbitReqStatus.ForeColor = Color.Lime;
                if (_logForm != null && !_logForm.IsDisposed)
                    _logForm.AddLog("[IBIT] IBIT_DONE 수신 완료, 결과 요청 대기 중");
            }));
        }

        private void OnCommError(string msg)
        {
            if (_logForm != null && !_logForm.IsDisposed)
            {
                _logForm.AddLog($"[ERROR] {msg}");
            }

            /* 통신 두절 표시 (슈드 UI 스레드 안전 처리) */
            if (lblCommStatus != null && !IsDisposed)
            {
                BeginInvoke((Action)(() =>
                {
                    lblCommStatus.Text = "⚠ " + msg;
                    lblCommReceiving.ForeColor = Color.Red;
                }));
            }
        }

        private DateTime _lastRxLogTime = DateTime.MinValue;

        private void OnRawRxReceived(byte[] packet)
        {
            // 초당 500번(2ms 주기) 들어오는 Reflect 패킷으로 인한 C# UI 뻗음(Freeze) 완벽 방지
            // 100ms 이하 간격의 원시 로그는 UI에 그리지 않고 스킵합니다. (데이터 처리는 정상 동작함)
            if ((DateTime.Now - _lastRxLogTime).TotalMilliseconds < 100) return;
            _lastRxLogTime = DateTime.Now;

            BeginInvoke((Action)(() =>
            {
                string hex = BitConverter.ToString(packet).Replace("-", " ");
                lblLogRxInfo.Text = $"RX: {hex}";
                if (_logForm != null && !_logForm.IsDisposed) _logForm.AddLog($"RX: {hex}");
            }));
        }

        private void OnRawTxReceived(byte[] packet)
        {
            BeginInvoke((Action)(() =>
            {
                string hex = BitConverter.ToString(packet).Replace("-", " ");
                lblLogTxInfo.Text = $"TX: {hex}";
                if (_logForm != null && !_logForm.IsDisposed) _logForm.AddLog($"TX: {hex}");
            }));
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            // 통신 수신 인디케이터 관리 (500ms 무응답 시 회색)
            if ((DateTime.Now - _lastRxTime).TotalMilliseconds > 500)
            {
                lblCommReceiving.ForeColor = Color.Gray;
            }
        }

        // Custom UI Control Helpers
        private Panel CreateStyledPanel(string title)
        {
            Panel p = new Panel { BackColor = colorPanelBg, BorderStyle = BorderStyle.FixedSingle };
            Label l = new Label
            {
                Text = title,
                ForeColor = colorAccent,
                Font = new Font("Segoe UI", 12, FontStyle.Bold),
                Dock = DockStyle.Top,
                Height = 35,
                TextAlign = ContentAlignment.MiddleLeft,
                Padding = new Padding(10, 0, 0, 0),
                BackColor = Color.FromArgb(35, 35, 38)
            };
            p.Controls.Add(l);
            return p;
        }

        private Button CreateBorderedButton(string text, int x, int y, int w, int h)
        {
            Button btn = new Button
            {
                Text = text,
                Location = new Point(x, y),
                Size = new Size(w, h),
                FlatStyle = FlatStyle.Flat,
                Font = new Font("맑은 고딕", 10, FontStyle.Bold),
                BackColor = Color.FromArgb(45, 45, 48),
                ForeColor = Color.White,
                Cursor = Cursors.Hand
            };
            btn.FlatAppearance.BorderColor = colorAccent;
            btn.FlatAppearance.BorderSize = 1;
            btn.FlatAppearance.MouseOverBackColor = Color.FromArgb(60, 60, 65);
            return btn;
        }
    }
}
