#!/usr/bin/env python
"""

Measurement topology for EECS489, Winter 2017, Assignment 1
"""

from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel
from mininet.node import OVSController

class AssignmentNetworks(Topo):
    def __init__(self, **opts):
        Topo.__init__(self, **opts)
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')
        h5 = self.addHost('h5')
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        s5 = self.addSwitch('s5')
        self.addLink(h1, s1)
        self.addLink(h2, s2)
        self.addLink(h5, s5)
        self.addLink(h3, s3)
        self.addLink(h4, s4)
    
        self.addLink(s1, s2, bw=20, delay='10ms')
        self.addLink(s2, s3, bw=40, delay='10ms')
        self.addLink(s2, s4, bw=40, delay='30ms')
        self.addLink(s2, s5, bw=45, delay='5ms')
        
        
        
if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = AssignmentNetworks()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True, controller = OVSController)

    # Run network
    net.start()
    CLI( net )
    net.stop()

