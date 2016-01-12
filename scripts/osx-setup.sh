#!/usr/bin/bash
# With the exception of XCode, this script will install all of CalChart's
# dependencies on an OSX machine. This script is intended to be run on
# an OSX version 10.10 or later. XCode should be installed before this
# script is run. 
# This script should be executed with one of the following commands:
#     . osx-setup.sh
#     source osx-setup.sh

# Determines whether or not a function is installed to path
# To use, pass the name of a program to this function
# It will succeed if the program is not installed to path
# It will fail otherwise
function not_on_path() {
	return $([ -z "$(type $1 2>/dev/null)" ])
}

# Navigate to project root directory
# Technique for finding script directory is described here: http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
script_directory=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
cd ${script_directory}/..

# Install XCode command line tools, if not installed
# Succeeds if the command line tools are installed successfully
# Fails otherwise
function install_xcode_command_line_tools() {
	# See http://stackoverflow.com/questions/15371925/how-to-check-if-command-line-tools-is-installed
	# We will assume that all of our Mac developers are using OSX 10.10 or later
	if [ `xcode-select -p` ] ; then
		echo "XCode command line tools already installed."
	else
	    echo "Installing XCode command line tools..."
	    xcode-select --install
	fi
	if [ ! `xcode-select -p` ] ; then
		echo "Error while installing XCode line tools."
		return 1
	fi
	return 0
}

# Install MacPorts, if not installed
# Succeeds if MacPorts is installed successfully
# Fails otherwise
function install_macports() {
	if `not_on_path port` ; then
		# Make sure that PATH contains the folders which would contain the `port` program
		PATH=/opt/local/bin:/opt/local/sbin:$PATH
	fi
	if `not_on_path port` ; then
		echo "Installing MacPorts..."

		# Make a temporary directory for downloading files into
		rm -rf temp
		mkdir temp
		cd temp
		
		# Download and install
		curl -O https://distfiles.macports.org/MacPorts/MacPorts-2.3.3.tar.bz2
	    tar xf MacPorts-2.3.3.tar.bz2
	    cd MacPorts-2.3.3/
	    ./configure
	    make
	    sudo make install

	    # Remove temporary directory
	    cd ..
	    cd ..
	    rm -rf temp
	else
	    echo "MacPorts already installed."
	fi
	if `not_on_path port` ; then
		echo "Error while installing MacPorts."
		return 1
	fi
	return 0
}

# Install Boost, if not installed
# Succeeds if Boost is installed successfully
# Fails otherwise
function install_boost() {
	echo "Installing Boost..."
	sudo port selfupdate
	sudo port install boost
	return 0
}

# Install wxWidgets, if not installed
# Succeeds if wxWidgets is installed successfully
# Fails otherwise
function install_wxwidgets() {
	wxwidgets_vers=3.0.2
	if `not_on_path wx-config` ; then
	    echo "Installing wxWidgets..."
	    
	    # Check to make sure that an OSX SDK is available
	    sdk_dir=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
	    all_sdks=$(ls ${sdk_dir} | grep ".sdk$")
	    if [ $? -ne 0 ] ; then
	        echo "Could not find OSX SDK; please install XCode."
	        exit 1
	    fi

	    # Select an SDK from those that are available
	    num_sdks=$(echo ${all_sdks} | wc -l)
	    if [ ${num_sdks} -gt 1 ] ; then
	        echo "Multiple SDKs available. This script will not choose one for you. Aborting..."
	        exit 1
	    else
	    	selected_sdk=${all_sdks}
	    fi

	    # Make a temporary directory for downloading files
	    rm -rf temp
	    mkdir temp
	    cd temp


	    # Use Vagrant to download wxWidgets, since it has the `wget` program installed
	    vagrant up
	    vagrant ssh -c "wget -O temp/wxWidgets-${wxwidgets_vers}.tar.bz2 https://github.com/wxWidgets/wxWidgets/releases/download/v${wxwidgets_vers}/wxWidgets-${wxwidgets_vers}.tar.bz2"
	    tar xf wxWidgets-${wxwidgets_vers}.tar.bz2
	    cd wxWidgets-${wxwidgets_vers}

	    # Patch wxWidgets (http://trac.wxwidgets.org/ticket/16329)
		curl http://trac.wxwidgets.org/raw-attachment/ticket/16329/wx_webview.patch > wx_webview.patch
		patch -p0 < wx_webview.patch 
		
		# Build and install wxWidgets
		mkdir build-results
		cd build-results
		../configure --with-cocoa --with-macosx-version-min=10.7 --with-macosx-sdk=${sdk_dir}/${selected_sdk} --enable-debug --enable-debug_info --disable-shared CXXFLAGS="-std=c++11 -stdlib=libc++" OBJCXXFLAGS="-stdlib=libc++ -std=c++11" LDFLAGS=-stdlib=libc++
		make -j4
		sudo make install

		# Remove temporary directory
		cd ..
		cd ..
		rm -rf temp
	else
	    echo "wxWidgets already installed."
	fi
	if `not_on_path wx-config` ; then
		echo "Error while installing wxWidgets."
		return 1
	fi
	return 0
}

# Build CalChart's generated files
# Succeeds if the files are generated successfully
# Fails otherwise
function build_generated_files() {
	echo "Building CalChart's generated files..."
	vagrant up
	vagrant ssh -c "make generate"
	return 0
}

install_xcode_command_line_tools && 
install_macports && 
install_boost &&
install_wxwidgets &&
build_generated_files

if [ $? -eq 0 ] ; then
	echo "Finished successfully."
else
	echo "Aborted due to errors."
fi
