
using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace ClapReceiver
{
    public class Program
    {
        private static readonly Receiver Receiver = new Receiver();

        public static void Main(string[] args)
        {
            Receiver.StartListening();
            Console.ReadLine();
        }
    }

    public class Receiver
    {
        private readonly UdpClient udp;
        private IPEndPoint ip = new IPEndPoint(IPAddress.Any, 43210);
        public Receiver()
        {
            udp = new UdpClient
            {
                ExclusiveAddressUse = false
            };
            udp.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            udp.Client.Bind(ip);
        }

        public void StartListening()
        {
            udp.BeginReceive(Receive, new object());
        }
        private void Receive(IAsyncResult ar)
        {
            var bytes = udp.EndReceive(ar, ref ip);
            Console.WriteLine(bytes[4]);
            var foregroundWindow = GetForegroundWindow();
            if (bytes[4] == 1)
            {
                PostMessage(foregroundWindow, WmKeydown, VkSpace, 0);
            }
            if (bytes[4] == 2)
            {
                PostMessage(foregroundWindow, WmKeydown, VkLeft, 0);
            }
            StartListening();
        }

        private const uint WmKeydown = 0x0100;
        private const int VkSpace = 0x20;
        private const int VkLeft = 0x25;

        [DllImport("user32.dll")]
        private static extern bool PostMessage(IntPtr hWnd, uint msg, int wParam, int lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr GetForegroundWindow();

    }
}
