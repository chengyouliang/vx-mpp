/**
  @file src/components/alsa/omx_alsasrc_component.c

  OpenMAX ALSA source component. This component is an audio source that uses ALSA library.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies)

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

#include <omxcore.h>
#include <omx_base_audio_port.h>
#include "omx_alsasrc_component.h"

/** Maximum Number of AlsaSrc Instance*/
#define MAX_COMPONENT_ALSASRC 1

/** Number of AlsaSrc Instance*/
static OMX_U32 noAlsasrcInstance=0;

/** The Constructor
 */
OMX_ERRORTYPE omx_alsasrc_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {
  int err;
  int omxErr;
  omx_base_audio_PortType *pPort;
  omx_alsasrc_component_PrivateType* omx_alsasrc_component_Private;
  OMX_U32 i;

  if (!openmaxStandComp->pComponentPrivate) {
    openmaxStandComp->pComponentPrivate = calloc(1, sizeof(omx_alsasrc_component_PrivateType));
    if(openmaxStandComp->pComponentPrivate==NULL) {
      return OMX_ErrorInsufficientResources;
    }
  }

  omx_alsasrc_component_Private = openmaxStandComp->pComponentPrivate;
  omx_alsasrc_component_Private->ports = NULL;

  omxErr = omx_base_source_Constructor(openmaxStandComp,cComponentName);
  if (omxErr != OMX_ErrorNone) {
    return OMX_ErrorInsufficientResources;
  }

  omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
  omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 1;

  /** Allocate Ports and call port constructor. */
  if (omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_alsasrc_component_Private->ports) {
    omx_alsasrc_component_Private->ports = calloc(omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
    if (!omx_alsasrc_component_Private->ports) {
      return OMX_ErrorInsufficientResources;
    }
    for (i=0; i < omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) {
      omx_alsasrc_component_Private->ports[i] = calloc(1, sizeof(omx_base_audio_PortType));
      if (!omx_alsasrc_component_Private->ports[i]) {
        return OMX_ErrorInsufficientResources;
      }
    }
  }

  base_audio_port_Constructor(openmaxStandComp, &omx_alsasrc_component_Private->ports[0], 0, OMX_FALSE);

  pPort = (omx_base_audio_PortType *) omx_alsasrc_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];

  // set the pPort params, now that the ports exist
  /** Domain specific section for the ports. */
  pPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
  /*Input pPort buffer size is equal to the size of the output buffer of the previous component*/
  pPort->sPortParam.nBufferSize = DEFAULT_OUT_BUFFER_SIZE;

  omx_alsasrc_component_Private->BufferMgmtCallback = omx_alsasrc_component_BufferMgmtCallback;
  omx_alsasrc_component_Private->destructor = omx_alsasrc_component_Destructor;

  setHeader(&pPort->sAudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
  pPort->sAudioParam.nPortIndex = 0;
  pPort->sAudioParam.nIndex = 0;
  pPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;

  /* OMX_AUDIO_PARAM_PCMMODETYPE */
  setHeader(&omx_alsasrc_component_Private->sPCMModeParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
  omx_alsasrc_component_Private->sPCMModeParam.nPortIndex = 0;
  omx_alsasrc_component_Private->sPCMModeParam.nChannels = 2;
  omx_alsasrc_component_Private->sPCMModeParam.eNumData = OMX_NumericalDataSigned;
  omx_alsasrc_component_Private->sPCMModeParam.eEndian = OMX_EndianLittle;
  omx_alsasrc_component_Private->sPCMModeParam.bInterleaved = OMX_TRUE;
  omx_alsasrc_component_Private->sPCMModeParam.nBitPerSample = 16;
  omx_alsasrc_component_Private->sPCMModeParam.nSamplingRate = 8000;
  omx_alsasrc_component_Private->sPCMModeParam.ePCMMode = OMX_AUDIO_PCMModeLinear;
  omx_alsasrc_component_Private->sPCMModeParam.eChannelMapping[0] = OMX_AUDIO_ChannelNone;

  noAlsasrcInstance++;
  if(noAlsasrcInstance > MAX_COMPONENT_ALSASRC) {
    return OMX_ErrorInsufficientResources;
  }

  //char *device = "plughw:0,0"; /* default device */
  /* Allocate the playback handle and the hardware parameter structure */
  if ((err = snd_pcm_open (&omx_alsasrc_component_Private->playback_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    DEBUG(DEB_LEV_ERR, "cannot open audio device %s (%s)\n", "default", snd_strerror (err));
    return OMX_ErrorHardware;
  }
  else
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Got playback handle at %08x %08X in %i\n", omx_alsasrc_component_Private->playback_handle, &omx_alsasrc_component_Private->playback_handle, getpid());

  if (snd_pcm_hw_params_malloc(&omx_alsasrc_component_Private->hw_params) < 0) {
    DEBUG(DEB_LEV_ERR, "%s: failed allocating input pPort hw parameters\n", __func__);
    return OMX_ErrorHardware;
  }
  else
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Got hw parameters at %08x\n", omx_alsasrc_component_Private->hw_params);

  if ((err = snd_pcm_hw_params_any (omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params)) < 0) {
    DEBUG(DEB_LEV_ERR, "cannot initialize hardware parameter structure (%s)\n",  snd_strerror (err));
    return OMX_ErrorHardware;
  }

  openmaxStandComp->SetParameter  = omx_alsasrc_component_SetParameter;
  openmaxStandComp->GetParameter  = omx_alsasrc_component_GetParameter;

  /* Write in the default paramenters */
  omx_alsasrc_component_Private->AudioPCMConfigured  = 0;

  if (!omx_alsasrc_component_Private->AudioPCMConfigured) {
    DEBUG(DEB_LEV_SIMPLE_SEQ, "Configuring the PCM interface in the Init function\n");
    omxErr = omx_alsasrc_component_SetParameter(openmaxStandComp, OMX_IndexParamAudioPcm, &omx_alsasrc_component_Private->sPCMModeParam);
    if(omxErr != OMX_ErrorNone){
      DEBUG(DEB_LEV_ERR, "In %s Error %08x\n",__func__,omxErr);
    }
  }

  return OMX_ErrorNone;
}

/** The Destructor
 */
OMX_ERRORTYPE omx_alsasrc_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {
  omx_alsasrc_component_PrivateType* omx_alsasrc_component_Private = openmaxStandComp->pComponentPrivate;
  OMX_U32 i;

  if(omx_alsasrc_component_Private->hw_params) {
    snd_pcm_hw_params_free (omx_alsasrc_component_Private->hw_params);
  }
  if(omx_alsasrc_component_Private->playback_handle) {
    snd_pcm_close(omx_alsasrc_component_Private->playback_handle);
  }

  /* frees port/s */
  if (omx_alsasrc_component_Private->ports) {
    for (i=0; i < omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) {
      if(omx_alsasrc_component_Private->ports[i])
        omx_alsasrc_component_Private->ports[i]->PortDestructor(omx_alsasrc_component_Private->ports[i]);
    }
    free(omx_alsasrc_component_Private->ports);
    omx_alsasrc_component_Private->ports=NULL;
  }

  noAlsasrcInstance--;

  return omx_base_source_Destructor(openmaxStandComp);

}

/**
 * This function plays the input buffer. When fully consumed it returns.
 */
void omx_alsasrc_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* outputbuffer) {
  OMX_U32  frameSize;
  OMX_S32 data_read;
  omx_alsasrc_component_PrivateType* omx_alsasrc_component_Private = openmaxStandComp->pComponentPrivate;

  /* Feed it to ALSA */
  frameSize = (omx_alsasrc_component_Private->sPCMModeParam.nChannels * omx_alsasrc_component_Private->sPCMModeParam.nBitPerSample) >> 3;
  DEBUG(DEB_LEV_FULL_SEQ, "Framesize is %u chl=%d bufSize=%d\n",
  (int)frameSize, (int)omx_alsasrc_component_Private->sPCMModeParam.nChannels, (int)outputbuffer->nFilledLen);

  if(outputbuffer->nAllocLen < frameSize){
    DEBUG(DEB_LEV_ERR, "Ouch!! In %s input buffer filled len(%d) less than frame size(%d)\n",__func__, (int)outputbuffer->nFilledLen, (int)frameSize);
    return;
  }

  data_read = snd_pcm_readi(omx_alsasrc_component_Private->playback_handle,outputbuffer->pBuffer,outputbuffer->nAllocLen/frameSize);
  if (data_read<0) {
    if (data_read !=-EPIPE){
      DEBUG(DEB_LEV_ERR,"alsa_card_read 1: snd_pcm_readi() failed:%s.\n",snd_strerror(data_read));
    }
    snd_pcm_prepare(omx_alsasrc_component_Private->playback_handle);
    data_read=snd_pcm_readi(omx_alsasrc_component_Private->playback_handle,outputbuffer->pBuffer,outputbuffer->nAllocLen/frameSize);
    if (data_read<0) {
      DEBUG(DEB_LEV_ERR,"alsa_card_read 2: snd_pcm_readi() failed:%s.\n",snd_strerror(data_read));
      return;
    }
  }

  outputbuffer->nFilledLen =  data_read*frameSize;

  DEBUG(DEB_LEV_FULL_SEQ, "Data read=%d, framesize=%d, o/b filled len=%d alloclen=%d\n",(int)data_read,(int)frameSize,(int)outputbuffer->nFilledLen,(int)outputbuffer->nAllocLen);

}

OMX_ERRORTYPE omx_alsasrc_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure)
{
  int err;
  int omxErr = OMX_ErrorNone;
  OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
  OMX_U32 portIndex;

  /* Check which structure we are being fed and make control its header */
  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
  omx_alsasrc_component_PrivateType* omx_alsasrc_component_Private = openmaxStandComp->pComponentPrivate;
  omx_base_audio_PortType* pPort = (omx_base_audio_PortType *) omx_alsasrc_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];
  snd_pcm_t* playback_handle = omx_alsasrc_component_Private->playback_handle;
  snd_pcm_hw_params_t* hw_params = omx_alsasrc_component_Private->hw_params;

  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }

  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);

  /** Each time we are (re)configuring the hw_params thing
  * we need to reinitialize it, otherwise previous changes will not take effect.
  * e.g.: changing a previously configured sampling rate does not have
  * any effect if we are not calling this each time.
  */
  err = snd_pcm_hw_params_any (omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params);

  switch(nParamIndex) {
  case OMX_IndexParamAudioPortFormat:
    pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
    portIndex = pAudioPortFormat->nPortIndex;
    /*Check Structure Header and verify component state*/
    omxErr = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    if(omxErr!=OMX_ErrorNone) {
      DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, omxErr);
      break;
    }
    if (portIndex < 1) {
      memcpy(&pPort->sAudioParam,pAudioPortFormat,sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    } else {
      return OMX_ErrorBadPortIndex;
    }
    break;
  case OMX_IndexParamAudioPcm:
    {
      unsigned int rate;
      OMX_AUDIO_PARAM_PCMMODETYPE* sPCMModeParam = (OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure;

      portIndex = sPCMModeParam->nPortIndex;
      /*Check Structure Header and verify component state*/
      omxErr = omx_base_component_ParameterSanityCheck(hComponent, portIndex, sPCMModeParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
      if(omxErr!=OMX_ErrorNone) {
        DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n", __func__, omxErr);
        break;
      }

      omx_alsasrc_component_Private->AudioPCMConfigured  = 1;
      if(sPCMModeParam->nPortIndex != omx_alsasrc_component_Private->sPCMModeParam.nPortIndex){
        DEBUG(DEB_LEV_ERR, "Error setting input pPort index\n");
        omxErr = OMX_ErrorBadParameter;
        break;
      }

      DEBUG(DEB_LEV_PARAMS, "Debug: nCh=%d, sRate=%d, bIL=%x,ePCMMode=%x,bps=%d\n",
        (int)sPCMModeParam->nChannels,
        (int)sPCMModeParam->nSamplingRate,
        sPCMModeParam->bInterleaved,
        sPCMModeParam->ePCMMode,
        (int)sPCMModeParam->nBitPerSample);

      if(snd_pcm_hw_params_set_channels(playback_handle, hw_params, sPCMModeParam->nChannels)){
        DEBUG(DEB_LEV_ERR, "Error setting number of channels\n");
        return OMX_ErrorBadParameter;
      }

      if(sPCMModeParam->bInterleaved == OMX_TRUE){
        if ((err = snd_pcm_hw_params_set_access(omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
          DEBUG(DEB_LEV_ERR, "cannot set access type intrleaved (%s)\n", snd_strerror (err));
          return OMX_ErrorHardware;
        }
      }
      else{
        if ((err = snd_pcm_hw_params_set_access(omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED)) < 0) {
          DEBUG(DEB_LEV_ERR, "cannot set access type non interleaved (%s)\n", snd_strerror (err));
          return OMX_ErrorHardware;
        }
      }
      rate = sPCMModeParam->nSamplingRate;
      if ((err = snd_pcm_hw_params_set_rate_near(omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params, &rate, 0)) < 0) {
        DEBUG(DEB_LEV_ERR, "cannot set sample rate (%s)\n", snd_strerror (err));
        return OMX_ErrorHardware;
      }
      else{
        DEBUG(DEB_LEV_PARAMS, "Set correctly sampling rate to %i\n", (int)omx_alsasrc_component_Private->sPCMModeParam.nSamplingRate);
      }

      if(sPCMModeParam->ePCMMode == OMX_AUDIO_PCMModeLinear){
        snd_pcm_format_t snd_pcm_format = SND_PCM_FORMAT_UNKNOWN;
        DEBUG(DEB_LEV_PARAMS, "Bit per sample %i, signed=%i, little endian=%i\n",
        (int)sPCMModeParam->nBitPerSample,
        (int)sPCMModeParam->eNumData == OMX_NumericalDataSigned,
        (int)sPCMModeParam->eEndian ==  OMX_EndianLittle);

        switch(sPCMModeParam->nBitPerSample){
        case 8:
          if(sPCMModeParam->eNumData == OMX_NumericalDataSigned) {
            snd_pcm_format = SND_PCM_FORMAT_S8;
          } else {
            snd_pcm_format = SND_PCM_FORMAT_U8;
          }
          break;
        case 16:
          if(sPCMModeParam->eNumData == OMX_NumericalDataSigned){
            if(sPCMModeParam->eEndian ==  OMX_EndianLittle) {
              snd_pcm_format = SND_PCM_FORMAT_S16_LE;
            } else {
              snd_pcm_format = SND_PCM_FORMAT_S16_BE;
            }
          }
        if(sPCMModeParam->eNumData == OMX_NumericalDataUnsigned){
          if(sPCMModeParam->eEndian ==  OMX_EndianLittle){
            snd_pcm_format = SND_PCM_FORMAT_U16_LE;
          } else {
            snd_pcm_format = SND_PCM_FORMAT_U16_BE;
          }
        }
        break;
        case 24:
          if(sPCMModeParam->eNumData == OMX_NumericalDataSigned){
            if(sPCMModeParam->eEndian ==  OMX_EndianLittle) {
              snd_pcm_format = SND_PCM_FORMAT_S24_LE;
            } else {
              snd_pcm_format = SND_PCM_FORMAT_S24_BE;
            }
          }
          if(sPCMModeParam->eNumData == OMX_NumericalDataUnsigned){
            if(sPCMModeParam->eEndian ==  OMX_EndianLittle) {
              snd_pcm_format = SND_PCM_FORMAT_U24_LE;
            } else {
              snd_pcm_format = SND_PCM_FORMAT_U24_BE;
            }
          }
          break;

        case 32:
          if(sPCMModeParam->eNumData == OMX_NumericalDataSigned){
            if(sPCMModeParam->eEndian ==  OMX_EndianLittle) {
              snd_pcm_format = SND_PCM_FORMAT_S32_LE;
            } else {
              snd_pcm_format = SND_PCM_FORMAT_S32_BE;
            }
          }
          if(sPCMModeParam->eNumData == OMX_NumericalDataUnsigned){
            if(sPCMModeParam->eEndian ==  OMX_EndianLittle) {
              snd_pcm_format = SND_PCM_FORMAT_U32_LE;
            } else {
              snd_pcm_format = SND_PCM_FORMAT_U32_BE;
            }
          }
          break;
        default:
          omxErr = OMX_ErrorBadParameter;
          break;
        }

        if(snd_pcm_format != SND_PCM_FORMAT_UNKNOWN){
          if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
            DEBUG(DEB_LEV_ERR, "cannot set sample format (%s)\n",  snd_strerror (err));
            return OMX_ErrorHardware;
          }
          memcpy(&omx_alsasrc_component_Private->sPCMModeParam, ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        } else{
          DEBUG(DEB_LEV_SIMPLE_SEQ, "ALSA OMX_IndexParamAudioPcm configured\n");
          memcpy(&omx_alsasrc_component_Private->sPCMModeParam, ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        }
      }
      else if(sPCMModeParam->ePCMMode == OMX_AUDIO_PCMModeALaw){
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Configuring ALAW format\n\n");
        if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_A_LAW)) < 0) {
          DEBUG(DEB_LEV_ERR, "cannot set sample format (%s)\n",  snd_strerror (err));
          return OMX_ErrorHardware;
        }
        memcpy(&omx_alsasrc_component_Private->sPCMModeParam, ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
      }
      else if(sPCMModeParam->ePCMMode == OMX_AUDIO_PCMModeMULaw){
        DEBUG(DEB_LEV_SIMPLE_SEQ, "Configuring ALAW format\n\n");
        if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_MU_LAW)) < 0) {
          DEBUG(DEB_LEV_ERR, "cannot set sample format (%s)\n", snd_strerror (err));
          return OMX_ErrorHardware;
        }
        memcpy(&omx_alsasrc_component_Private->sPCMModeParam, ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
      }

      /** Configure and prepare the ALSA handle */
      DEBUG(DEB_LEV_SIMPLE_SEQ, "Configuring the PCM interface\n");
      if ((err = snd_pcm_hw_params (omx_alsasrc_component_Private->playback_handle, omx_alsasrc_component_Private->hw_params)) < 0) {
        DEBUG(DEB_LEV_ERR, "cannot set parameters (%s)\n",  snd_strerror (err));
        return OMX_ErrorHardware;
      }

      if ((err = snd_pcm_prepare (omx_alsasrc_component_Private->playback_handle)) < 0) {
        DEBUG(DEB_LEV_ERR, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
        return OMX_ErrorHardware;
      }
    }
    break;
  default: /*Call the base component function*/
    return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
  }
  return omxErr;
}

OMX_ERRORTYPE omx_alsasrc_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure)
{
  OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE*)hComponent;
  omx_alsasrc_component_PrivateType* omx_alsasrc_component_Private = openmaxStandComp->pComponentPrivate;
  omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_alsasrc_component_Private->ports[OMX_BASE_SOURCE_OUTPUTPORT_INDEX];
  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);
  /* Check which structure we are being fed and fill its header */
  switch(nParamIndex) {
  case OMX_IndexParamAudioInit:
    if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) {
      break;
    }
    memcpy(ComponentParameterStructure, &omx_alsasrc_component_Private->sPortTypesParam[OMX_PortDomainAudio], sizeof(OMX_PORT_PARAM_TYPE));
    break;
  case OMX_IndexParamAudioPortFormat:
    pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;
    if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) {
      break;
    }
    if (pAudioPortFormat->nPortIndex < 1) {
      memcpy(pAudioPortFormat, &pPort->sAudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    } else {
      return OMX_ErrorBadPortIndex;
    }
    break;
  case OMX_IndexParamAudioPcm:
    if(((OMX_AUDIO_PARAM_PCMMODETYPE*)ComponentParameterStructure)->nPortIndex !=
      omx_alsasrc_component_Private->sPCMModeParam.nPortIndex) {
      return OMX_ErrorBadParameter;
    }
    if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone) {
      break;
    }
    memcpy(ComponentParameterStructure, &omx_alsasrc_component_Private->sPCMModeParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
    break;
  default: /*Call the base component function*/
  return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
  }
  return err;
}
