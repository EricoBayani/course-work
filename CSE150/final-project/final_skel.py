#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
from mininet.cli import CLI
from mininet.node import RemoteController

class final_topo(Topo):
  def build(self):
    # Examples!
    # Create a host with a default route of the ethernet interface. You'll need to set the
    # default gateway like this for every host you make on this assignment to make sure all 
    # packets are sent out that port. Make sure to change the h# in the defaultRoute area
    # and the MAC address when you add more hosts!
    # h1 = self.addHost('h1',mac='00:00:00:00:00:01',ip='1.1.1.1/24', defaultRoute="h1-eth0")
    # h2 = self.addHost('h2',mac='00:00:00:00:00:02',ip='2.2.2.2/24', defaultRoute="h2-eth0")

    # Create a switch. No changes here from Lab 1.
    # s1 = self.addSwitch('s1')

    # Connect Port 8 on the Switch to Port 0 on Host 1 and Port 9 on the Switch to Port 0 on 
    # Host 2. This is representing the physical port on the switch or host that you are 
    # connecting to.
    #
    # IMPORTANT NOTES: 
    # - On a single device, you can only use each port once! So, on s1, only 1 device can be
    #   plugged in to port 1, only one device can be plugged in to port 2, etc.
    # - On the "host" side of connections, you must make sure to always match the port you 
    #   set as the default route when you created the device above. Usually, this means you 
    #   should plug in to port 0 (since you set the default route to h#-eth0).
    #
    # self.addLink(s1,h1, port1=8, port2=0)
    # self.addLink(s1,h2, port1=9, port2=0)

    print "Delete me!"
    print "I refuse to delete you" 
    
    h10 = self.addHost('h10',mac='00:00:00:00:00:10',ip='10.1.1.10/24', defaultRoute="h10-eth0")
    h20 = self.addHost('h20',mac='00:00:00:00:00:20',ip='10.1.2.20/24', defaultRoute="h20-eth0")
    h30 = self.addHost('h30',mac='00:00:00:00:00:30',ip='10.1.3.30/24', defaultRoute="h30-eth0")
    h40 = self.addHost('h40',mac='00:00:00:00:00:40',ip='10.1.4.40/24', defaultRoute="h40-eth0")

    h50 = self.addHost('h50',mac='00:00:00:00:00:50',ip='10.2.5.50/24', defaultRoute="h50-eth0")
    h60 = self.addHost('h60',mac='00:00:00:00:00:60',ip='10.2.6.60/24', defaultRoute="h60-eth0")
    h70 = self.addHost('h70',mac='00:00:00:00:00:70',ip='10.2.7.70/24', defaultRoute="h70-eth0")
    h80 = self.addHost('h80',mac='00:00:00:00:00:80',ip='10.2.8.80/24', defaultRoute="h80-eth0")

    h_trust = self.addHost('h_trust',mac='00:00:00:00:00:A0',ip='108.24.31.112/24', defaultRoute="h_trust-eth0")

    h_untrust = self.addHost('h_untrust',mac='00:00:00:00:00:B0',ip='106.44.82.103/24', defaultRoute="h_untrust-eth0")

    h_server = self.addHost('h_server',mac='00:00:00:00:FF:00',ip='10.3.9.90/24', defaultRoute="h_server-eth0")

    f1_s1 = self.addSwitch('s1') # floor 1 switch 1
    f1_s2 = self.addSwitch('s2') # floor 1 switch 2

    f2_s1 = self.addSwitch('s3') # floor 2 switch 1
    f2_s2 = self.addSwitch('s4') # floor 2 switch 2

    core_switch = self.addSwitch('s5') # core switch
    data_center_switch = self.addSwitch('s6') # switch to server

    self.addLink(f1_s1, h10, port1=8, port2=0)
    self.addLink(f1_s1, h20, port1=9, port2=0)
    self.addLink(f1_s2, h30, port1=8, port2=0)
    self.addLink(f1_s2, h40, port1=9, port2=0)

    self.addLink(f2_s1, h50, port1=8, port2=0)
    self.addLink(f2_s1, h60, port1=9, port2=0)
    self.addLink(f2_s2, h70, port1=8, port2=0)
    self.addLink(f2_s2, h80, port1=9, port2=0)

    self.addLink(core_switch, f1_s1, port1=8, port2=1)
    self.addLink(core_switch, f1_s2, port1=9, port2=1)

    self.addLink(core_switch, f2_s1, port1=10, port2=1)
    self.addLink(core_switch, f2_s2, port1=11, port2=1)

    self.addLink(core_switch, h_trust, port1=12, port2=0)
    self.addLink(core_switch, h_untrust, port1=13, port2=0)

    self.addLink(core_switch, data_center_switch, port1=14, port2=1)

    self.addLink(data_center_switch, h_server, port1=8, port2=0)


def configure():
  topo = final_topo()
  net = Mininet(topo=topo, controller=RemoteController)
  net.start()

  CLI(net)
  
  net.stop()


if __name__ == '__main__':
  configure()
