//
//  mp4record.c
//  RTSP_Player
//
//  Created by apple on 15/4/7.
//  Copyright (c) 2015年 thinker. All rights reserved.
//


#include "mp4record.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#define H264_START_CODE 0x000001

typedef struct MP4V2_CONTEXT{
    
    int m_vWidth,m_vHeight,m_vFrateR,m_vTimeScale;
    MP4FileHandle m_mp4FHandle;
    MP4TrackId m_vTrackId,m_aTrackId;
    double m_vFrameDur;
    
} MP4V2_CONTEXT;

struct MP4V2_CONTEXT * recordCtx = NULL;


int initMp4Encoder(const char * filename,int width,int height){
    
    int ret = -1;
    recordCtx = malloc(sizeof(struct MP4V2_CONTEXT));
    if (!recordCtx) {
        printf("error : malloc context \n");
        return ret;
    }
    
    recordCtx->m_vWidth = width;
    recordCtx->m_vHeight = height;
    recordCtx->m_vFrateR = 25;
    recordCtx->m_vTimeScale = 90000;
    recordCtx->m_vFrameDur = 300;
    recordCtx->m_vTrackId = 0;
    recordCtx->m_aTrackId = 0;
    
    recordCtx->m_mp4FHandle = MP4Create(filename,0);
    if (recordCtx->m_mp4FHandle == MP4_INVALID_FILE_HANDLE) {
        printf("error : MP4Create  \n");
        return ret;
    }
     MP4SetTimeScale(recordCtx->m_mp4FHandle, recordCtx->m_vTimeScale);
    //------------------------------------------------------------------------------------- audio track
//    recordCtx->m_aTrackId = MP4AddAudioTrack(recordCtx->m_mp4FHandle, 44100, 1024, MP4_MPEG4_AUDIO_TYPE);
//    if (recordCtx->m_aTrackId == MP4_INVALID_TRACK_ID){
//        printf("error : MP4AddAudioTrack  \n");
//        return ret;
//    }
//
//    MP4SetAudioProfileLevel(recordCtx->m_mp4FHandle, 0x2);
//    uint8_t aacConfig[2] = {18,16};
//    MP4SetTrackESConfiguration(recordCtx->m_mp4FHandle,recordCtx->m_aTrackId,aacConfig,2);
//    printf("ok  : initMp4Encoder file=%s  \n",filename);

    return 0;
}
int mp4VEncode(uint8_t * _naluData ,int _naluSize){
    
    int index = -1;
    
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x67){
        index = _NALU_SPS_;
    }
    
    if(index!=_NALU_SPS_ && recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
        return index;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x68){
        index = _NALU_PPS_;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x65){
        index = _NALU_I_;
    }
    if(_naluData[0]==0 && _naluData[1]==0 && _naluData[2]==0 && _naluData[3]==1 && _naluData[4]==0x41){
        index = _NALU_P_;
    }
    //
    switch(index){
        case _NALU_SPS_:
            if(recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
                recordCtx->m_vTrackId = MP4AddH264VideoTrack
                (recordCtx->m_mp4FHandle,
                 recordCtx->m_vTimeScale,
                 recordCtx->m_vTimeScale / recordCtx->m_vFrateR,
                 recordCtx->m_vWidth,     // width
                 recordCtx->m_vHeight,    // height
                 _naluData[5], // sps[1] AVCProfileIndication
                 _naluData[6], // sps[2] profile_compat
                 _naluData[7], // sps[3] AVCLevelIndication
                 3);           // 4 bytes length before each NAL unit
                if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID)  {
                    return -1;
                }
                MP4SetVideoProfileLevel(recordCtx->m_mp4FHandle, 0x7F); //  Simple Profile @ Level 3
            }
            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle,recordCtx->m_vTrackId,_naluData+4,_naluSize-4);
            //
            break;
        case _NALU_PPS_:
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle,recordCtx->m_vTrackId,_naluData+4,_naluSize-4);
            break;
        case _NALU_I_:
        {
            uint8_t * IFrameData = malloc(_naluSize+1);
            //
            IFrameData[0] = (_naluSize-3) >>24;
            IFrameData[1] = (_naluSize-3) >>16;
            IFrameData[2] = (_naluSize-3) >>8;
            IFrameData[3] = (_naluSize-3) &0xff;
    
            std::memcpy(IFrameData+4,_naluData+3,_naluSize-3);
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, MP4_INVALID_DURATION, 0, 1)){
                return -1;
            }
            free(IFrameData);
            //
            break;
        }
        case _NALU_P_:
        {
            _naluData[0] = (_naluSize-4) >>24;  
            _naluData[1] = (_naluSize-4) >>16;  
            _naluData[2] = (_naluSize-4) >>8;  
            _naluData[3] = (_naluSize-4) &0xff;
            
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, MP4_INVALID_DURATION, 0, 0)){
                return -1;
            }
            break;
        }
    }
    return 0;
}


int mp4AEncode(uint8_t * data ,int len){
    if(recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID){
        return -1;
    }
    MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_aTrackId, data, len , MP4_INVALID_DURATION, 0, 1);
    recordCtx->m_vFrameDur += 1024;
    return 0;
}

void closeMp4Encoder(){
    if(recordCtx){
        if (recordCtx->m_mp4FHandle != MP4_INVALID_FILE_HANDLE) {
            MP4Close(recordCtx->m_mp4FHandle,0);
            recordCtx->m_mp4FHandle = NULL;
        }
        
        free(recordCtx);
        recordCtx = NULL;
    }
    
    printf("ok  : closeMp4Encoder  \n");

}

uint32_t h264_find_next_start_code (uint8_t *pBuf, 
				    uint32_t bufLen)
{
  uint32_t val;
  uint32_t offset;

  offset = 0;
  if (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1) {
    pBuf += 4;
    offset = 4;
  } else if (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1) {
    pBuf += 3;
    offset = 3;
  }
  val = 0xffffffff;
  while (offset < bufLen - 3) {
    val <<= 8;
    val |= *pBuf++;
    offset++;
    if (val == H264_START_CODE) {
      return offset - 4;
    }
    if ((val & 0x00ffffff) == H264_START_CODE) {
      return offset - 3;
    }
  }
  return 0;
}

static uint32_t remove_03 (uint8_t *bptr, uint32_t len)
{
  uint32_t nal_len = 0;

  while (nal_len + 2 < len) {
    if (bptr[0] == 0 && bptr[1] == 0 && bptr[2] == 3) {
      bptr += 2;
      nal_len += 2;
      len--;
      std::memmove(bptr, bptr + 1, len - nal_len);
    } else {
      bptr++;
      nal_len++;
    }
  }
  return len;
}

int main (int argc, char **argv)
{
#define MAX_BUFFER 65536 * 8
  
  uint8_t buffer[MAX_BUFFER];
  uint32_t buffer_on, buffer_size;
  uint64_t bytes = 0;
  FILE *m_file;
  
  bool have_prevdec = false;
 
    m_file = fopen("test.h264", "rb");

  if (m_file == NULL) {
    fprintf(stderr, "file not found\n");
    exit(-1);
  }

  buffer_on = buffer_size = 0;
  while (!feof(m_file)) {
    bytes += buffer_on;
    if (buffer_on != 0) {
      buffer_on = buffer_size - buffer_on;
      std::memmove(buffer, &buffer[buffer_size - buffer_on], buffer_on);
    }
    buffer_size = fread(buffer + buffer_on, 
			1, 
			MAX_BUFFER - buffer_on, 
			m_file);
    buffer_size += buffer_on;
    buffer_on = 0;

    bool done = false;
    //CBitstream ourbs;
    do {
      uint32_t ret;
      ret = h264_find_next_start_code(buffer + buffer_on, 
				      buffer_size - buffer_on);
      if (ret == 0) {
	done = true;
	if (buffer_on == 0) {
	  fprintf(stderr, "couldn't find start code in buffer from 0\n");
	  exit(-1);
	}
      } else {
	// have a complete NAL from buffer_on to end
	if (ret > 3) {
	  uint32_t nal_len;

	  nal_len = remove_03(buffer + buffer_on, ret);

	  printf("Nal length %u start code %u bytes \n", nal_len, 
		 buffer[buffer_on + 2] == 1 ? 3 : 4);

#if 0
	  ourbs.init(buffer + buffer_on, nal_len * 8);
	  uint8_t type;
	  type = h264_parse_nal(&dec, &ourbs);
	  if (type >= 1 && type <= 5) {
	    if (have_prevdec) {
	      // compare the 2
	      bool bound;
	      bound = compare_boundary(&prevdec, &dec);
	      printf("Nal is %s\n", bound ? "part of last picture" : "new picture");
	    }
	    memcpy(&prevdec, &dec, sizeof(dec));
	    have_prevdec = true;
	  } else if (type >= 9 && type <= 11) {
	    have_prevdec = false; // don't need to check
	  }
#endif
	}
#if 1
	printf("buffer on \"X64\" %lu len %u %02x %02x %02x %02x\n",
	       bytes + buffer_on, 
	       bytes + buffer_on + ret,
	       buffer_on, 
	       ret,
	       buffer[buffer_on],
	       buffer[buffer_on+1],
	       buffer[buffer_on+2],
	       buffer[buffer_on+3]);
#endif
	buffer_on += ret; // buffer_on points to next code
      }
    } while (done == false);
  }
  fclose(m_file);
  return 0;
}
