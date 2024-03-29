/**
  @file src/components/kms/library_entry_point.c
  
  OpenMAX kms sink component. 

  Copyright (C) 2023-2024 vanxum

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
 
  $Date: 2023/3/23 +0530 (Tue, 02 Sep 2008) $
  Revision $Rev: 593 $
  Author $Author: chengyouliang $
*/



#include <st_static_component_loader.h>
#include <omx_kms_sink_component.h>

/** @brief The library entry point. It must have the same name for each
  * library of the components loaded by the ST static component loader.
  * 
  * This function fills the version, the component name and if existing also the roles
  * and the specific names for each role. This base function is only an explanation.
  * For each library it must be implemented, and it must fill data of any component
  * in the library.
  * 
  * @param stComponents pointer to an array of components descriptors.If NULL, the 
  * function will return only the number of components contained in the library
  * 
  * @return number of components contained in the library 
*/
OMX_S32 omx_component_library_Setup(stLoaderComponentType **stComponents) {
  OMX_S32 i;
  DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n",__func__);

  if (stComponents == NULL) {
    DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n",__func__);
    return 1; // Return Number of Components - one for kms sink component
  }

  /** component 1 - kms sink component */
  stComponents[0]->componentVersion.s.nVersionMajor = 1; 
  stComponents[0]->componentVersion.s.nVersionMinor = 1; 
  stComponents[0]->componentVersion.s.nRevision = 1;
  stComponents[0]->componentVersion.s.nStep = 1;

  stComponents[0]->name = calloc(1, OMX_MAX_STRINGNAME_SIZE);
  if (stComponents[0]->name == NULL) {
    return OMX_ErrorInsufficientResources;
  }
  strcpy(stComponents[0]->name, "OMX.st.video.kms_sink");
  stComponents[0]->name_specific_length = 1; 
  stComponents[0]->constructor = omx_kms_sink_component_Constructor;  

  stComponents[0]->name_specific = calloc(stComponents[0]->name_specific_length,sizeof(char *));  
  stComponents[0]->role_specific = calloc(stComponents[0]->name_specific_length,sizeof(char *));  

  for(i=0;i<stComponents[0]->name_specific_length;i++) {
    stComponents[0]->name_specific[i] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
    if (stComponents[0]->name_specific[i] == NULL) {
      return OMX_ErrorInsufficientResources;
    }
  }
  for(i=0;i<stComponents[0]->name_specific_length;i++) {
    stComponents[0]->role_specific[i] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
    if (stComponents[0]->role_specific[i] == NULL) {
      return OMX_ErrorInsufficientResources;
    }
  }
  strcpy(stComponents[0]->name_specific[0], "OMX.st.video.kmssink");
  strcpy(stComponents[0]->role_specific[0], "video.kmssink");

  DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n",__func__);
  return 1; 
}
