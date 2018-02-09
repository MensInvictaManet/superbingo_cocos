#ifndef _VIDEO_PLAY_H_
#define _VIDEO_PLAY_H_

extern void VideoInit();
extern void LoadVideoFile(const char* videoName);
extern void PlayVideoFile(bool repeat, int width, int height);
extern void StopVideoFile();


#endif // _VIDEO_PLAY_H_