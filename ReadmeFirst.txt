General
-------
This is the source archive of Link Shell Extension, which started in 1999, so 20 years from now. It has been
maintained a long time without source control and contains some projects
o) LinkShellExtension
     HardlinkShellExt.dll
	 LSEUacHelper.exe
	 LSEConfig.exe
o) ln.exe
o) Dupemerge.exe
o) xdel.exe (rarley used)
o) du.exe (internal tool)
o) TimeStamp.exe (internal tool, only used in the automated test of ln.exe)
o) Where.exe (internal tool)

Infrastructure
--------------
There are few tools needed to compile and regression test the suite
o) VS2017
o) Truecrypt.exe
o) Nsis-2.14 (really old...)

Compiling
---------
This should be fairly easy. Simply recompile the whole link.sln in all configurations

Testing
-------
A few things are needed
o) Installed truecrypt.exe
o) A profile in HardlinkExtension/Settings.bat for your machine. Copy an existing one
   and replace the needed entries. This should be fairly easy
o) If the binaries are compiled ln/LnAllTest.bat runs a test which takes about 15 minutes, and goes 
   through a lot of nifty testcases. By the end LnTest_good.txt and LnTest_Current should be almost the same
   Just at the end in the json test there is little difference, which does not harm. This is just an order topic
   
Install Media
-------------
Install Media are created by BuildWorld.bat. To specify the version number edit BuildWorld.bat first.

Please also take note of the comment in buildworld.bat about changing the registry schema. Normally 
nothing is needed here.

There is an NSIS Setup for Hardlink Shell Ext with a custom .dll to handle all the nifty parts like killing
processes, which hold hardlinkshellext.dll. See HardlinkExtension\install\setup-processes. There are two extra solutions
to compile this helper. Please see HardlinkExtension\install\setup-processes\TheBigPicture.txt.

All output can be found in the Media/ directory

There is HardlinkShellExt/install/ReleaseCheckList.txt to think of all things to be done.

The nsis-installer is signed with a self signed certificate from LinkShellExtension.pfx, which is also provided
If the orginator Hermann Schinagl code signs the project, there is a verified full blown code signing certificate to sign it. 
This certificate is not freely avaiable
   
Documentation
-------------
HardlinkExtension/Doc holds a .html file and lots of pictures to outline things
ln/Doc holds a .html file and lots of pictures to outline things
DupeMerge/Doc holds a .html file and lots of pictures to outline things

Distribution
------------
If you want to create a new distro
o) Edit BuildWorld.bat for the version and rarley for the registry schema
o) Follow the instructions under HardlinkExtension\install\setup-processes\TheBigPicture.txt if you need to recompile processes.dll
   You have to do this once, when you start from a clean git-clone. If you have the .dll you can skip this step
o) Run BuildWorld.bat. As part of it recompile everyhing <you are told>
o) Run LnAlltest.bat and compare LnTest_Current.txt with LnTest_Good.txt
o) Continue in BuildWorld.bat to create the Media/
o) Upload media and .html to hermanns website
o) Upload lse.xml so that the RSS feed contains a new entry
o) Post to facebook

Status
------
Currently I am trying to clean up things, so that the dust of the past is removed from the coding
and things are more automated. Well this takes. There are a few small issues, well need to adress them.

License
--------
The source license is missing. But it will be a GPL style license. For now it has none, which means the source must not be copied.
The license for binaries can be found on the website and basically says, that you can take the binaries and do what you like.


