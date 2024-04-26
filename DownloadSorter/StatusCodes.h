#pragma once


#ifndef STATUSCODES_H
#define STATUSCODES_H


#define D_SUCCESS					0x0
#define D_ERROR						0x1
#define D_BASEDIR_NOT_DIRECTORY		0x2
#define D_BASEDIR_NOT_FOUND			0x3	
#define D_USERDIR_NOT_LOCATED		0x4	// critical
#define D_USERNAME					0x5	// critical
#define D_DOWNLOADPATH_NOT_FOUND	0x6 // critical
#define D_CONFIGFILE_NOT_FOUND      0x7 
#define D_SETTINGS_FILE_NOT_FOUND	0x8

#endif // !STATUSCODES_H
