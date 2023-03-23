/**
  @file src/st_static_component_loader.c

  ST specific component loader for local components.

  Copyright (C) 2007, 2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <strings.h>
#include <errno.h>
#include <assert.h>

#include "common.h"
#include "st_static_component_loader.h"
#include "omx_base_component.h"

/** This pointer holds and handle allocate by this loader and requested by
 * some application. If the IL client does not de-allocate it calling
 * explicitely the FreeHandle function, any pending handle is release at the
 * end, when the global function OMX_Deinit is called. This list takes track of
 * any handle
 */
void *handleLibList[100];
/** Current number of handles already allocated by this loader
 */
OMX_U32 numLib=0;

/** @brief The initialization of the ST specific component loader.
 *
 * This function allocates memory for the component loader and initialize other function pointer
 */
void st_static_setup_component_loader(BOSA_COMPONENTLOADER * st_static_loader) {
	  st_static_loader->BOSA_InitComponentLoader = &BOSA_ST_InitComponentLoader;
	  st_static_loader->BOSA_DeInitComponentLoader = &BOSA_ST_DeInitComponentLoader;
	  st_static_loader->BOSA_CreateComponent = &BOSA_ST_CreateComponent;
	  st_static_loader->BOSA_DestroyComponent = &BOSA_ST_DestroyComponent;
	  st_static_loader->BOSA_ComponentNameEnum = &BOSA_ST_ComponentNameEnum;
	  st_static_loader->BOSA_GetRolesOfComponent = &BOSA_ST_GetRolesOfComponent;
	  st_static_loader->BOSA_GetComponentsOfRole = &BOSA_ST_GetComponentsOfRole;

}
/** @brief the ST static loader contructor
 *
 * This function creates the ST static component loader, and creates
 * the list of available components, based on a registry file
 * created by a separate appication. It is called omxregister,
 * and must be called before the use of this loader
 */
OMX_ERRORTYPE BOSA_ST_InitComponentLoader(BOSA_COMPONENTLOADER *loader) {
  int num_of_comp=0;
  int read;
  stLoaderComponentType** templateList;
  stLoaderComponentType** stComponentsTemp;
  void* handle;
  int (*fptr)(stLoaderComponentType **stComponents);
  int i;
  int index;
  int listindex;
  DIR *dir = NULL;
	struct dirent *entry;
	char *dir_path = NULL;
  char path[1024];
  if (loader == NULL)
	{
		  DEBUG(DEB_LEV_ERR, "please input loader is null\n");
      return -1;
	}
	dir_path = registryGetDir();
	if (dir_path == NULL)
	{
		  DEBUG(DEB_LEV_ERR, "please set up component lib dir OMX_VX_REGISTRY \n");
      return -1;
	}
  listindex = 0;
  templateList = malloc(sizeof (stLoaderComponentType*));
  templateList[0] = NULL;

	dir = opendir(dir_path);
  if(dir == NULL) {
	    DEBUG(DEB_LEV_ERR, "open dir err %s  %d\n", path, dlerror());
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
        printf("%s %d %s\n",__FUNCTION__,__LINE__,path);
        handle = NULL;
		    handle = dlopen(path, RTLD_NOW);
	      if (!handle)
		    {
			      DEBUG(DEB_LEV_ERR, "library %s dlopen error: %s\n", path, dlerror());
			      continue;
	      }
        printf("%s %d %s\n",__FUNCTION__,__LINE__,path);
        handleLibList[numLib]=handle;
        numLib++;
        if ((fptr = dlsym(handle, "omx_component_library_Setup")) == NULL) {
          
            DEBUG(DEB_LEV_ERR, "the library %s is not compatible with ST static component loader - %s\n", path, dlerror());
        } else {
            num_of_comp = (int)(*fptr)(NULL);
            printf("%s %d\n",__FUNCTION__,__LINE__);
            templateList = realloc(templateList, (listindex + num_of_comp + 1) * sizeof (stLoaderComponentType*));
            templateList[listindex + num_of_comp] = NULL;
            printf("%s %d\n",__FUNCTION__,__LINE__);
            stComponentsTemp = calloc(num_of_comp,sizeof(stLoaderComponentType*));
            for (i = 0; i<num_of_comp; i++) {
              stComponentsTemp[i] = calloc(1,sizeof(stLoaderComponentType));
            }
            printf("%s %d\n",__FUNCTION__,__LINE__);
            (*fptr)(stComponentsTemp);
            printf("%s %d\n",__FUNCTION__,__LINE__);
            for (i = 0; i<num_of_comp; i++) {
              templateList[listindex + i] = stComponentsTemp[i];
              DEBUG(DEB_LEV_FULL_SEQ, "In %s comp name[%d]=%s\n",__func__,listindex + i,templateList[listindex + i]->name);
            }
            free(stComponentsTemp);
            stComponentsTemp = NULL;
            listindex+= i;
      }
  }
  loader->loaderPrivate = templateList;
  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  closedir(dir);
  return OMX_ErrorNone;
}

/** @brief The destructor of the ST specific component loader.
 *
 * This function deallocates the list of available components.
 */
OMX_ERRORTYPE BOSA_ST_DeInitComponentLoader(BOSA_COMPONENTLOADER *loader) {
  unsigned int i, j;
  int err;
  stLoaderComponentType** templateList;
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
  templateList = (stLoaderComponentType**)loader->loaderPrivate;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

  i = 0;
  while(templateList[i]) {
    if(templateList[i]->name_requested){
      free(templateList[i]->name_requested);
      templateList[i]->name_requested=NULL;
    }

    for(j = 0 ; j < templateList[i]->name_specific_length; j++){
      if(templateList[i]->name_specific[j]) {
        free(templateList[i]->name_specific[j]);
        templateList[i]->name_specific[j]=NULL;
      }
      if(templateList[i]->role_specific[j]){
        free(templateList[i]->role_specific[j]);
        templateList[i]->role_specific[j]=NULL;
      }
    }

    if(templateList[i]->name_specific){
      free(templateList[i]->name_specific);
      templateList[i]->name_specific=NULL;
    }
    if(templateList[i]->role_specific){
      free(templateList[i]->role_specific);
      templateList[i]->role_specific=NULL;
    }
    if(templateList[i]->name){
      free(templateList[i]->name);
      templateList[i]->name=NULL;
    }
    free(templateList[i]);
    templateList[i] = NULL;
    i++;
  }
  if(templateList) {
    free(templateList);
    templateList=NULL;
  }

  for(i=0;i<numLib;i++) {
    err = dlclose(handleLibList[i]);
    if(err!=0) {
      DEBUG(DEB_LEV_ERR, "In %s Error %d in dlclose of lib %i\n", __func__,err,i);
    }
  }
  numLib=0;

  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  return OMX_ErrorNone;
}

/** @brief creator of the requested OpenMAX component
 *
 * This function searches for the requested component in the internal list.
 * If the component is found, its constructor is called,
 * and the standard callbacks are assigned.
 * A pointer to a standard OpenMAX component is returned.
 */
OMX_ERRORTYPE BOSA_ST_CreateComponent(
  BOSA_COMPONENTLOADER *loader,
  OMX_HANDLETYPE* pHandle,
  OMX_STRING cComponentName,
  OMX_PTR pAppData,
  OMX_CALLBACKTYPE* pCallBacks) {

  int i = 0;
  unsigned int j;
  int componentPosition = -1;
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  stLoaderComponentType** templateList;
  OMX_COMPONENTTYPE *openmaxStandComp;
  omx_base_component_PrivateType * priv;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
  templateList = (stLoaderComponentType**)loader->loaderPrivate;
  i = 0;
  while(templateList[i]) {
    DEBUG(DEB_LEV_PARAMS, "In %s\n", __func__,templateList[i]->name,cComponentName);
    if(!strcmp(templateList[i]->name, cComponentName)) {
      //given component name matches with the general component names
      componentPosition = i;
      break;
    } else {
      for(j=0;j<templateList[i]->name_specific_length;j++) {
        if(!strcmp(templateList[i]->name_specific[j], cComponentName)) {
          //given component name matches with specific component names
          componentPosition = i;
          break;
        }
      }
      if(componentPosition != -1) {
        break;
      }
    }
    i++;
  }
  if (componentPosition == -1) {
    DEBUG(DEB_LEV_ERR, "Component not found with current ST static component loader.\n");
    return OMX_ErrorComponentNotFound;
  }

  //component name matches with general component name field
  DEBUG(DEB_LEV_PARAMS, "Found base requested template %s\n", cComponentName);
  /* Build ST component from template and fill fields */
  templateList[componentPosition]->name_requested = strndup (cComponentName, OMX_MAX_STRINGNAME_SIZE);

  openmaxStandComp = calloc(1,sizeof(OMX_COMPONENTTYPE));
  if (!openmaxStandComp) {
    return OMX_ErrorInsufficientResources;
  }
  eError = templateList[componentPosition]->constructor(openmaxStandComp,cComponentName);
  if (eError != OMX_ErrorNone) {
    if (eError == OMX_ErrorInsufficientResources) {
      *pHandle = openmaxStandComp;
      priv = (omx_base_component_PrivateType *) openmaxStandComp->pComponentPrivate;
      priv->loader = loader;
      return OMX_ErrorInsufficientResources;
    }
    DEBUG(DEB_LEV_ERR, "Error during component construction\n");
    openmaxStandComp->ComponentDeInit(openmaxStandComp);
    free(openmaxStandComp);
    openmaxStandComp = NULL;
    return OMX_ErrorComponentNotFound;
  }
  priv = (omx_base_component_PrivateType *) openmaxStandComp->pComponentPrivate;
  priv->loader = loader;

  *pHandle = openmaxStandComp;
  ((OMX_COMPONENTTYPE*)*pHandle)->SetCallbacks(*pHandle, pCallBacks, pAppData);
  DEBUG(DEB_LEV_FULL_SEQ, "Template %s found returning from OMX_GetHandle\n", cComponentName);
  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  return eError;
}

OMX_ERRORTYPE BOSA_ST_DestroyComponent(
  BOSA_COMPONENTLOADER *loader,
  OMX_HANDLETYPE hComponent) {
  OMX_ERRORTYPE err = OMX_ErrorNone;
  omx_base_component_PrivateType * priv = (omx_base_component_PrivateType *) ((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate;

  /* check if this component was actually loaded from this loader */
  if (priv->loader != loader) {
    return OMX_ErrorComponentNotFound;
  }

  err = ((OMX_COMPONENTTYPE*)hComponent)->ComponentDeInit(hComponent);

  free((OMX_COMPONENTTYPE*)hComponent);
  hComponent = NULL;

  return err;
}

/** @brief This function search for the index from 0 to end of the list
 *
 * This function searches in the list of ST static components and enumerates
 * both the class names and the role specific components.
 */
OMX_ERRORTYPE BOSA_ST_ComponentNameEnum(
  BOSA_COMPONENTLOADER *loader,
  OMX_STRING cComponentName,
  OMX_U32 nNameLength,
  OMX_U32 nIndex) {

  stLoaderComponentType** templateList;
  int i;
  unsigned int j, index = 0;
  int found = 0;
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

  templateList = (stLoaderComponentType**)loader->loaderPrivate;
  i = 0;
  while(templateList[i]) {
    if (index == nIndex) {
      strncpy(cComponentName, templateList[i]->name, nNameLength);
      found = 1;
      break;
    }
    index++;
    if (templateList[i]->name_specific_length > 0) {
      for (j = 0; j<templateList[i]->name_specific_length; j++) {
        if (index == nIndex) {
          strncpy(cComponentName,templateList[i]->name_specific[j], nNameLength);
          found = 1;
          break;
        }
        index++;
      }
    }
    if (found) {
      break;
    }
    i++;
  }
  if (!found) {
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s with OMX_ErrorNoMore\n", __func__);
    return OMX_ErrorNoMore;
  }
  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  return OMX_ErrorNone;
}

/** @brief The specific version of OMX_GetRolesOfComponent
 *
 * This function replicates exactly the behavior of the
 * standard OMX_GetRolesOfComponent function for the ST static
 * component loader
 */
OMX_ERRORTYPE BOSA_ST_GetRolesOfComponent(
  BOSA_COMPONENTLOADER *loader,
  OMX_STRING compName,
  OMX_U32 *pNumRoles,
  OMX_U8 **roles) {

  stLoaderComponentType** templateList;
  int i;
  unsigned int j, index;
  unsigned int max_roles = *pNumRoles;
  int found = 0;
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
  templateList = (stLoaderComponentType**)loader->loaderPrivate;
  *pNumRoles = 0;
  i = 0;
  while (templateList[i]) {
    if(!strcmp(templateList[i]->name, compName)) {
      DEBUG(DEB_LEV_SIMPLE_SEQ, "Found requested template %s IN GENERAL COMPONENT\n", compName);
      // set the no of roles field
      *pNumRoles = templateList[i]->name_specific_length;
      if(roles == NULL) {
        return OMX_ErrorNone;
      }
      //append the roles
      for (index = 0; index < templateList[i]->name_specific_length; index++) {
        if (index < max_roles) {
          strcpy ((char*)*(roles+index), templateList[i]->role_specific[index]);
        }
      }
      found = 1;
    } else {
      for(j=0;j<templateList[i]->name_specific_length;j++) {
        if(!strcmp(templateList[i]-> name_specific[j], compName)) {
          DEBUG(DEB_LEV_SIMPLE_SEQ, "Found requested component %s IN SPECIFIC COMPONENT \n", compName);
          *pNumRoles = 1;
          found = 1;
          if(roles == NULL) {
            return OMX_ErrorNone;
          }
          if (max_roles > 0) {
            strcpy ((char*)*roles , templateList[i]->role_specific[j]);
          }
        }
      }
    }
    i++;
    if(found) {
      break;
    }
  }
  if(!found) {
    DEBUG(DEB_LEV_ERR, "no component match in whole template list has been found\n");
    *pNumRoles = 0;
    return OMX_ErrorComponentNotFound;
  }
  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  return OMX_ErrorNone;
}

/** @brief The specific version of OMX_GetComponentsOfRole
 *
 * This function replicates exactly the behavior of the
 * standard OMX_GetComponentsOfRole function for the ST static
 * component loader
 */
OMX_ERRORTYPE BOSA_ST_GetComponentsOfRole (
  BOSA_COMPONENTLOADER *loader,
  OMX_STRING role,
  OMX_U32 *pNumComps,
  OMX_U8  **compNames) {

  stLoaderComponentType** templateList;
  int i = 0;
  unsigned int j = 0;
  int num_comp = 0;
  int max_entries = *pNumComps;

  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);
  templateList = (stLoaderComponentType**)loader->loaderPrivate;
  i = 0;
  while(templateList[i]) {
    for (j = 0; j<templateList[i]->name_specific_length; j++) {
      if (!strcmp(templateList[i]->role_specific[j], role)) {
        if (compNames != NULL) {
          if (num_comp < max_entries) {
            strcpy((char*)(compNames[num_comp]), templateList[i]->name);
          }
        }
      num_comp++;
      }
    }
    i++;
  }

  *pNumComps = num_comp;
  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
  return OMX_ErrorNone;
}
