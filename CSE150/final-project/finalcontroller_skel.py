# Final Skeleton
#
# Hints/Reminders from Lab 3:
#
# To check the source and destination of an IP packet, you can use
# the header information... For example:
#
# ip_header = packet.find('ipv4')
#
# if ip_header.srcip == "1.1.1.1":
#   print "Packet is from 1.1.1.1"
#
# Important Note: the "is" comparison DOES NOT work for IP address
# comparisons in this way. You must use ==.
# 
# To send an OpenFlow Message telling a switch to send packets out a
# port, do the following, replacing <PORT> with the port number the 
# switch should send the packets out:
#
#    msg = of.ofp_flow_mod()
#    msg.match = of.ofp_match.from_packet(packet)
#    msg.idle_timeout = 30
#    msg.hard_timeout = 30
#
#    msg.actions.append(of.ofp_action_output(port = <PORT>))
#    msg.data = packet_in
#    self.connection.send(msg)
#
# To drop packets, simply omit the action.
#

from pox.core import core
import pox.openflow.libopenflow_01 as of
#imports taken from l2_learning.py as reference
#taken from the manual
import pox.lib.packet as pkt
from pox.lib.util import dpid_to_str
from pox.lib.util import str_to_bool
import time

log = core.getLogger()

class Final (object):
  """
  A Firewall object is created for each switch that connects.
  A Connection object for that switch is passed to the __init__ function.
  """
  def __init__ (self, connection):
    # Keep track of the connection to the switch so that we can
    # send it messages!
    self.connection = connection

    # This binds our PacketIn event listener
    connection.addListeners(self)

  def do_final (self, packet, packet_in, port_on_switch, switch_id):
    # This is where you'll put your code. The following modifications have 
    # been made from Lab 3:
    #   - port_on_switch: represents the port that the packet was received on.
    #   - switch_id represents the id of the switch that received the packet.
    #      (for example, s1 would have switch_id == 1, s2 would have switch_id == 2, etc...)
    # You should use these to determine where a packet came from. To figure out where a packet 
    # is going, you can use the IP header information.


    # This code has been directly lifted from my lab3 as a template

    def flood (duration = 60):
      """ Floods the packet, create a rule to flood similar packets """
      print "flooding"
      msg = of.ofp_packet_out()
      msg.actions.append(of.ofp_action_output(port = of.OFPP_FLOOD))
      msg.data = packet_in
      msg.in_port = packet_in.in_port
      self.connection.send(msg)

    def drop (duration = 60):
      """
      Drops this packet 
      """
      if duration is not None:
        if not isinstance(duration, tuple):
          duration = (duration,duration)
      msg = of.ofp_packet_out()
      #msg.in_port = packet_in.in_port
      #msg.buffer_id = packet_in.buffer_id
      msg.actions.append(of.ofp_action_output(port = of.OFPP_NONE))
      self.connection.send(msg)

    def forward (port):
      """ Floods the packet, create a rule to flood similar packets """
      print ("forwarding to {}".format(port))
      msg = of.ofp_packet_out()
      msg.actions.append(of.ofp_action_output(port = port))
      msg.data = packet_in
      msg.in_port = packet_in.in_port
      self.connection.send(msg)

   # print pkt.ETHERNET.ethernet.getNameForType(packet.type)
    #print packet_in
    
    def keepTCP():
      msg = of.ofp_flow_mod()
      msg.priorty = 100 # higher priority indicated by higer number
      #msg.idle_timeout = 5
      #msg.hard_timeout = 5
      match = of.ofp_match()
      match.dl_type = 0x800 # for IP
      match.nw_proto = 6 # for TCP
      msg.match = match
      msg.actions.append(of.ofp_action_output(port = of.OFPP_FLOOD))
      self.connection.send(msg)
    def keepARP():
      msg = of.ofp_flow_mod()
      msg.priorty = 2000 # higher priority indicated by higer number
      #msg.idle_timeout = 5
      #msg.hard_timeout = 5
      match = of.ofp_match()
      match.dl_type = 0x806 # for ARP
      msg.match = match
      msg.actions.append(of.ofp_action_output(port = of.OFPP_FLOOD))
      self.connection.send(msg)
      
    def keepForwardingIP(in_port, out_port, src_ip, dst_ip, priority = 100):
      msg = of.ofp_flow_mod()
      msg.priorty = priority # higher priority indicated by higer number
      #msg.idle_timeout = 5
      #msg.hard_timeout = 5
      match = of.ofp_match()
      match.dl_type = 0x800 # for IP
      msg.match = match
      match.in_port = in_port
      match.nw_src = src_ip
      match.nw_dst = dst_ip
      msg.actions.append(of.ofp_action_output(port = out_port))
      self.connection.send(msg)


    def dropElse():
      msg = of.ofp_flow_mod()
      msg.priorty = 10 # higher priority indicated by higer number
      match = of.ofp_match()
      msg.match = match
      msg.actions.append(of.ofp_action_output(port = of.OFPP_NONE))
      self.connection.send(msg)
    def dropICMP(in_port, dst_ip, priority):
      msg = of.ofp_flow_mod()
      msg.priorty = priority # higher priority indicated by higer number
      #msg.idle_timeout = 5
      #msg.hard_timeout = 5
      match = of.ofp_match()
      msg.match = match
      match.dl_type = 0x800 # for IP
      match.nw_proto = 1 # for ICMP
      match.in_port = in_port # the port it arrived in
      match.nw_dst = dst_ip # the port its going
      msg.actions.append(of.ofp_action_output(port = of.OFPP_NONE))
      self.connection.send(msg)
    def dropIP(in_port, nw_dst, priority = 100):
      msg = of.ofp_flow_mod()
      msg.priorty = priority # higher priority indicated by higer number
      #msg.idle_timeout = 5
      #msg.hard_timeout = 5
      match = of.ofp_match()
      msg.match = match
      match.dl_type = 0x800 # for IP
      match.in_port = in_port # the port it arrived in
      match.nw_dst = nw_dst
      msg.actions.append(of.ofp_action_output(port = of.OFPP_NONE))
      self.connection.send(msg)
      
    print "=========="

    #if ip is not None:
    ip = packet.find('ipv4')
    tcp = packet.find('tcp')
    icmp = packet.find('icmp')
    arp = packet.find('arp')
    
    # behaviours for different switches defined as fucntions
    # I'm using a dictionary to make a sort of switch statement in python
    """
    The switch mappings from their name in the topology description to their sequential id is as follows
    floor 1 switch 1 = s1
    floor 1 switch 2 = s2
    floor 2 switch 1 = s3
    floor 2 switch 2 = s4
    
    core switch = s5

    data center switch = s6

    the core switch ports: 
    port 8 <-> f1s1
    port 9 <-> f1s2
    port 10 <-> f2s1
    port 11 <-> f2s2
    port 12 <-> trust
    port 13 <-> untrust
    port 14 <-> datacenter
    """
    def s1():
      print "handling floor 1switch 1 traffic" 
      if ip is not None:
        switch_ports = {
          "10.1.1.10" : 8,
          "10.1.2.20" : 9,
          "10.1.3.30" : 1,
          "10.1.4.40" : 1,
          "10.2.5.50" : 1,
          "10.2.6.60" : 1,
          "10.2.7.70" : 1,
          "10.2.8.80" : 1,
          "108.24.31.112" : 1,
          "106.44.82.103" : 1,
          "10.3.9.90" : 1,

      }
        dstip_str = str(ip.dstip)
        out_port = switch_ports.get(dstip_str, 69)
        keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
        forward(out_port)
        print out_port
      else:
        flood()
        keepARP()
    def s2():
      print "handling floor 1 switch 2 traffic" 
      if ip is not None:
        switch_ports = {
          "10.1.1.10" : 1,
          "10.1.2.20" : 1,
          "10.1.3.30" : 8,
          "10.1.4.40" : 9,
          "10.2.5.50" : 1,
          "10.2.6.60" : 1,
          "10.2.7.70" : 1,
          "10.2.8.80" : 1,
          "108.24.31.112" : 1,
          "106.44.82.103" : 1,
          "10.3.9.90" : 1,
          
        }
        dstip_str = str(ip.dstip)
        out_port = switch_ports.get(dstip_str, 69)
        keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)      
        forward(out_port)
        print out_port
      else:
        flood()
        keepARP()
    def s3():
      print "handling floor 2 switch 1 traffic" 
      if ip is not None:
        switch_ports = {
          "10.1.1.10" : 1,
          "10.1.2.20" : 1,
          "10.1.3.30" : 1,
          "10.1.4.40" : 1,
          "10.2.5.50" : 8,
          "10.2.6.60" : 9,
          "10.2.7.70" : 1,
          "10.2.8.80" : 1,
          "108.24.31.112" : 1,
          "106.44.82.103" : 1,
          "10.3.9.90" : 1,
          
        }
        dstip_str = str(ip.dstip)
        out_port = switch_ports.get(dstip_str, 69)
        keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
        forward(out_port)
        print out_port
      else:
        flood()
        keepARP()
    def s4():
      print "handling floor 2 switch 2 traffic" 
      if ip is not None:
        switch_ports = {
          "10.1.1.10" : 1,
          "10.1.2.20" : 1,
          "10.1.3.30" : 1,
          "10.1.4.40" : 1,
          "10.2.5.50" : 1,
          "10.2.6.60" : 1,
          "10.2.7.70" : 8,
          "10.2.8.80" : 9,
          "108.24.31.112" : 1,
          "106.44.82.103" : 1,
          "10.3.9.90" : 1,
          
        }
        dstip_str = str(ip.dstip)
        out_port = switch_ports.get(dstip_str, 69)
        keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
        forward(out_port)
        print out_port
      else:
        flood()
        keepARP()
    def s5():
      print "handling core switch traffic" 
      core_switch_ports = {
        "10.1.1.10" : 8,
        "10.1.2.20" : 8,
        "10.1.3.30" : 9,
        "10.1.4.40" : 9,
        "10.2.5.50" : 10,
        "10.2.6.60" : 10,
        "10.2.7.70" : 11,
        "10.2.8.80" : 11,
        "108.24.31.112" : 12,
        "106.44.82.103" : 13,
        "10.3.9.90" : 14,
        
      }
      if ip is not None:
        dstip_str = str(ip.dstip)
        out_port = core_switch_ports.get(dstip_str, 69)
        if port_on_switch == 14: # switch to server          
          keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
          forward(out_port)
          print out_port
        elif port_on_switch == 13: # untrustedhost
          if icmp is not None:
            if ip.dstip == "108.24.31.112":
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              forward(out_port)
            else:
              dropICMP(port_on_switch, ip.dstip, 1000)
              drop()
              
          elif ip.dstip == "10.3.9.90":
            dropIP(port_on_switch, ip.dstip,900)
            drop()
          else:            
            keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
            forward(out_port)
            print out_port
        elif port_on_switch == 12: # trusted host
            if ip.dstip == "10.3.9.90":
              dropIP(port_on_switch, ip.dstip,900)
              drop()
            else:
            
              if out_port == 8 or out_port == 9 or out_port == 13:
                keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 1000)
                print out_port
                forward(out_port)
              else:
                dropICMP(port_on_switch, ip.dstip, 1000)
                drop()
        elif port_on_switch == 8: # switches to floor1             
          if icmp is not None:
            if out_port == 14 or out_port == 12 or out_port == 9:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              forward(out_port)
              print out_port
            else:
              dropICMP(port_on_switch, ip.dstip, 200)
              drop()
          else:
            keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
            print out_port                
        elif port_on_switch == 9: # switches to floor1            
          if icmp is not None:
            if out_port == 14 or out_port == 12 or out_port == 8:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              forward(out_port)
              print out_port
            else:
              dropICMP(port_on_switch, ip.dstip, 200)
              drop()
          else:
            keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
            print out_port
        elif port_on_switch == 10: # switches to floor2            
          if icmp is not None:
            if out_port == 14 or out_port == 11:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              forward(out_port)
              print out_port
            else:
              dropICMP(port_on_switch, ip.dstip, 200)
              drop()
          else:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              print out_port
        elif port_on_switch == 11: # switches to floor2            
          if icmp is not None:
            if out_port == 14 or out_port == 10:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              forward(out_port)
              print out_port
            else:
              dropICMP(port_on_switch, ip.dstip, 200)
              drop()
          else:
              keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
              print out_port
      else:
        flood()
        keepARP()
    def s6():
      print "handling data center switch traffic" 
      if ip is not None:
        data_center_switch_ports = {
          "10.1.1.10" : 1,
          "10.1.2.20" : 1,
          "10.1.3.30" : 1,
          "10.1.4.40" : 1,
          "10.2.5.50" : 1,
          "10.2.6.60" : 1,
          "10.2.7.70" : 1,
          "10.2.8.80" : 1,
          "108.24.31.112" : 1,
          "106.44.82.103" : 1,
          "10.3.9.90" : 8,
          
        }
        dstip_str = str(ip.dstip)
        out_port = data_center_switch_ports.get(dstip_str, 69)
        keepForwardingIP(port_on_switch, out_port, ip.srcip, ip.dstip, 500)
        forward(out_port)
        print out_port
      else:
        flood()
        keepARP()

    def noswitch():
      print "this switch doesn't exist, or is wrong" 

    

  
    switches = {1 : s1,
                 2 : s2,
                 3 : s3,
                 4 : s4,
                 5 : s5,
                 6 : s6,
                 69 : noswitch
    }
      
    switches.get(switch_id, noswitch)()

    print "=========="

  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch.
    """
    packet = event.parsed # This is the parsed packet data.
    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp # The actual ofp_packet_in message.
    self.do_final(packet, packet_in, event.port, event.dpid)

def launch ():
  """
  Starts the component
  """
  def start_switch (event):
    log.debug("Controlling %s" % (event.connection,))
    Final(event.connection)
  core.openflow.addListenerByName("ConnectionUp", start_switch)
