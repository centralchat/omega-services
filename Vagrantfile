# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!

Vagrant.configure(2) do |config|

  config.berkshelf.enabled = false
  config.omnibus.chef_version = :latest

  config.vm.box = "ubuntu12"

  config.vm.define :server_omega do |app|

    app.vm.provision :chef_solo do |chef|
      chef.run_list = []
    end

  end

end

