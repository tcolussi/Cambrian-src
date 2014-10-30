SocietyPro
======================

Installation instructions for Windows
=====================================

 * Initialize submodule
 git clone --recursive https://github.com/SocietyPro/Cambrian-src

 * Install the latest version of OpenSSL  
 Download the version recomended for developers from here:  
 http://slproweb.com/products/Win32OpenSSL.html  
 For example [Win32 OpenSSL v1.0.1j] - [16Mb]  
 
 Install it to the following path:  
 (or install it to any directory on your drive and copy that folder)
	
	sopro-cpp-dependencies\OpenSSL-Win32
	
	add "lib" prefix to the file ssleay32.a  
	sopro-cpp-dependencies\win32\OpenSSL-Win32\lib\MinGW\ssleay32.a (original name)  
	sopro-cpp-dependencies\win32\OpenSSL-Win32\lib\MinGW\libssleay32.a (renamed)  
	
 * Compile otx  
 Open the otx\projects\otx.pro, compile everything