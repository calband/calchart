# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
    # Build the virtual machine from Ubuntu 14.04.3 LTS (Trusty), 32 Bit
    config.vm.box = "ubuntu/trusty32"

    # Share the CalChart repository with the virtual machine
    config.vm.synced_folder ".", "/vagrant"

    # Provision the virtual machine using a setup script
    config.vm.provision :shell, :path => "vagrant/init.sh"
end
