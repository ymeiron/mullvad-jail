This is a guide to achieve jailing (or "selective lockdown mode") with Mullvad on Linux. In lockdown mode, all applications are blocked when Mullvad is disconnected. Using this method, only selected applications will be locked down and lose internet connectivity if they cannot reach Mullvad.

Run the following commands as root, change the 192.168.1.0/24 subnet if needed.

```
ip netns add mullvad-jail
ip link add mullvad-veth0 type veth peer name mullvad-veth1
ip link set mullvad-veth1 netns mullvad-jail
ip netns exec mullvad-jail ip addr add 192.168.1.1/24 dev mullvad-veth1
ip netns exec mullvad-jail ip link set mullvad-veth1 up
ip addr add 192.168.1.2/24 dev mullvad-veth0
ip link set mullvad-veth0 up
ip netns exec mullvad-jail ip route add default via 192.168.1.2 dev mullvad-veth1
iptables -t nat -A POSTROUTING -s 192.168.1.2/24 -o wg-mullvad -j MASQUERADE
ip netns exec mullvad-jail sysctl net.ipv4.ping_group_range="0 2147483647"
```

What happens here is:

* Creating a new network namespace called `mullvad-jail`
* Creating two connected virtual Ethernet devices (`mullvad-veth0`, `mullvad-veth1`)
* Moving `mullvad-veth1` into the `mullvad-jail` network namespace
* Setting up network addresses for them and bringing them up
* Setting up routing in the `mullvad-jail` network namespace
* Masquerading traffic from the network namespace behind `wg-mullvad`
* Configuring a kernel parameter to allow a normal user to ping (optional)

These commands need to run every time the system starts, so it's recommended to create a systemd service unit for this.

After setting up the `mullvad-jail` network namespace, the root user can run any command by prefixing it by `ip netns exec mullvad-jail`, that will cause all traffic to go through the `wg-mullvad` interface; if it is not available, traffic is firewalled. To run any command as ordinary user `user`, you can run `ip netns exec mullvad-jail su user -` as root and get a shell within the network namespace, so every application run from within will be similarly jailed. You can also build the program in this directory and install like so:

```
gcc mullvad-jail.c -o mullvad-jail
sudo chown root:root mullvad-jail
sudo chmod u+s mullvad-jail
sudo mv mullvad-jail /usr/local/bin/
```

Then, run any program as a normal user: for example, running `mullvad-jail firefox` will start the web browser, but internet traffic will only be possible trough `wg-mullvad`.