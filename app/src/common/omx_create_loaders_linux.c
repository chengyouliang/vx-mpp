/**
  @file src/omx_create_loaders_linux.c

  In this file is implemented the entry point for the construction
  of every component loader in linux. In the current implementation
  only the st static loader is called.

  Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

  $Date$
  Revision $Rev$
  Author $Author$

*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "component_loader.h"
#include "omx_create_loaders.h"
#include "st_static_component_loader.h"
#include "common.h"

#define PATH_LENGTH     1024     

int createComponentLoaders() {
	// load component loaders
	BOSA_COMPONENTLOADER *loader;
	void *handle;
	void *functionPointer;
	void (*fptr)(BOSA_COMPONENTLOADER *loader);
	int oneAtLeast = 0;
	DIR *dir = NULL;
	struct dirent *entry;
    struct stat sta;
	char *dir_path;
    char path[PATH_LENGTH];

	dir_path = registryGetDir();
	loader = NULL;
	dir = opendir(dir_path);
    if(dir == NULL) {
		DEBUG(DEB_LEV_ERR, "please set up lib dir  OMX_VX_REGISTRY", path, dlerror());
        return -1;
    }
	while(1) 
    {
		entry = readdir(dir);

        if(entry == NULL) {
            break;
        }

        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        memset(path, 0, sizeof(path));
        strcpy(path, dir_path);
        if(path[strlen(path) - 1] != '/') {
            strcat(path, "/");
        }

		strcat(path, entry->d_name);
		handle = dlopen(path, RTLD_NOW);
	    if (!handle)
		{
			DEBUG(DEB_LEV_ERR, "library %s dlopen error: %s\n", path, dlerror());
			continue;
	    }

	    if ((functionPointer = dlsym(handle, "setup_component_loader")) == NULL)
		{
			DEBUG(DEB_LEV_ERR, "the library %s is not compatible - %s\n", path, dlerror());
			continue;
		}
	    fptr = functionPointer;

		loader = calloc(1, sizeof(BOSA_COMPONENTLOADER));

		if (loader == NULL)
		{
				DEBUG(DEB_LEV_ERR, "not enough memory for this loader\n");
				return OMX_ErrorInsufficientResources;
		}

		/* setup the function pointers */
		(*fptr)(loader);

		/* add loader to core */
		BOSA_AddComponentLoader(loader);
		oneAtLeast = 1;
	}
	if (!oneAtLeast) {
		st_static_setup_component_loader(loader);
		BOSA_AddComponentLoader(loader);
	}
	closedir(dir);
	return 0;
}
