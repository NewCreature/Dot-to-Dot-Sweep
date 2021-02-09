#!/bin/bash

if [[ $# -ne 1 ]]; then
	SDK_FILE=MacOSX10.13
else
	SDK_FILE=$1
fi

rm -rf /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
rm -rf /Library/Developer/CommandLineTools/SDKs/MacOSX10.6.sdk
rm -rf /Library/Developer/CommandLineTools/SDKs/MacOSX10.13.sdk
rm -rf /Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk
rm -rf /Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk
ln -s /Volumes/MacOSExternal/Development/MacOS/SDKs/$SDK_FILE.sdk /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
ln -s /Volumes/MacOSExternal/Development/MacOS/SDKs/$SDK_FILE.sdk /Library/Developer/CommandLineTools/SDKs/$SDK_FILE.sdk
rm -rf /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
rm -rf /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk
rm -rf /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk
rm -rf /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.0.sdk
rm -rf /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk
ln -s /Volumes/MacOSExternal/Development/MacOS/SDKs/$SDK_FILE.sdk /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
ln -s /Volumes/MacOSExternal/Development/MacOS/SDKs/$SDK_FILE.sdk /Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/$SDK_FILE.sdk
