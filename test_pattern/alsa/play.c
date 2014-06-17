#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define AUDIO_ALSA_DBG(_fmt, _args...) printf(_fmt, ##_args)

int main(int argc, char** argv)
{
	snd_pcm_t *handle = NULL;
	snd_pcm_hw_params_t *params = NULL;
	snd_pcm_uframes_t frames = 64;
	int dir = 0, size, loops;
	unsigned int val;
	int ret = 0, n = 0;
	unsigned char *buf = NULL;
	FILE *fpcm = NULL;
	unsigned int rate;
	int format;
	int channel;
	int lfile;
	int write_size;
/*
	int i;
	for(i = 0; i < argc; i++)
		AUDIO_ALSA_DBG("param%d:		%s\n", i, *(argv + i));
*/

	if(6 != argc)
	{
		AUDIO_ALSA_DBG("\n");
		AUDIO_ALSA_DBG("param1 snd dev:			alsa device name: default, plughw:0,0 or plughw:1,0\n");
		AUDIO_ALSA_DBG("param2 filename:		pcm data file\n");
		AUDIO_ALSA_DBG("param3 sampleRate:		8000 -- 48000 \n");
		AUDIO_ALSA_DBG("param4 channel: 		1:mono, 2:stereo\n");
		AUDIO_ALSA_DBG("param5 format:			1:SND_PCM_FORMAT_U8,  2:SND_PCM_FORMAT_S16_LE\n\n");
		return -1;
	}
	
	rate = strtoul(*(argv + 3), NULL, 10);
	channel = strtoul(*(argv + 4), NULL, 10);
	format = strtoul(*(argv + 5), NULL, 10);

	if(rate < 8000 || rate > 48000)
	{
		AUDIO_ALSA_DBG("param2  sampleRate  error:       8000 -- 48000 \n");
		ret = -1;
		goto error;
	}

	if(channel < 1 || channel > 2)
	{
		AUDIO_ALSA_DBG("param3  channel  error:            1:mono, 2:stereo\n");
		ret = -1;
		goto error;
	}

	if(format < SND_PCM_FORMAT_U8 || format > SND_PCM_FORMAT_S16_LE)
	{
		AUDIO_ALSA_DBG("param4  format  error:             1:SND_PCM_FORMAT_U8,  2:SND_PCM_FORMAT_S16_LE\n");
		ret = -1;
		goto error;
	}

	AUDIO_ALSA_DBG("rate = %u    channel = %s    format = %s\n", rate, (1 == channel) ? "mono" : "stereo", 
		(1 == format) ? "SND_PCM_FORMAT_U8" : "SND_PCM_FORMAT_S16_LE");

	fpcm = fopen(*(argv + 2), "rb");
	if(!fpcm)
	{
		AUDIO_ALSA_DBG("Can not open file %s  ... \n", *(argv + 2));
		goto error;
	}

	fseek(fpcm,0,SEEK_END);
	lfile = ftell(fpcm);
	AUDIO_ALSA_DBG("file size is %d\n", lfile);
	fseek(fpcm,0,SEEK_SET);

	if((ret = snd_pcm_open(&handle, *(argv + 1), SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
	{
		AUDIO_ALSA_DBG("snd_pcm_open error: %s\n", snd_strerror(ret));
		handle = NULL;
		goto error;
	}

	snd_pcm_hw_params_malloc(&params);
	if(!params)
	{
		AUDIO_ALSA_DBG("snd_pcm_hw_params_alloca error ... \n");
		ret = -1;
		goto error;
	}

	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

	if((ret = snd_pcm_hw_params_set_format(handle ,params, format)) < 0)
	{
		AUDIO_ALSA_DBG("format set error: %s\n", snd_strerror(ret));
		goto error;
	}

	if((ret = snd_pcm_hw_params_set_channels(handle, params, channel)) < 0)
	{
		AUDIO_ALSA_DBG("channel set error: %s\n", snd_strerror(ret));
		goto error;
	}

	if((ret = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir)) < 0)
	{
		AUDIO_ALSA_DBG("sampleRate set error: %s\n", snd_strerror(ret));
		goto error;
	}


	if((ret = snd_pcm_hw_params(handle, params)) < 0)
	{
		AUDIO_ALSA_DBG("snd_pcm_hw_params error: %s\n", snd_strerror(ret));
		goto error;
	}
	
	n = 256;
	size = channel * format * n;

	buf = (unsigned char*)malloc(size);
	if(!buf)
	{
		AUDIO_ALSA_DBG("malloc buf error: buf size = %d\n", size);
		ret = -1;
		goto error;
	}

	while(lfile > size)
	{

		fread(buf, 1, size, fpcm);
		write_size = snd_pcm_writei(handle, buf, n);
		if(n != write_size)
		{
			AUDIO_ALSA_DBG("snd_pcm_writei error write_size:[%d], size:[%d]\n", write_size, n);
		}

		lfile = lfile - size;
	}
	if(lfile > 0)
	{
		fread(buf, 1, lfile, fpcm);
		snd_pcm_writei(handle, buf, lfile / (channel * format));
	}

error:
	if(params)
		snd_pcm_hw_params_free(params);

	if(handle)
		snd_pcm_close(handle);

	if(buf)
		free(buf);
	
	if(fpcm)
		fclose(fpcm);
		
	return ret;
}

