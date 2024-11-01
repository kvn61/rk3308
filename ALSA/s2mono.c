
#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>

#include <unistd.h>

#define DBG(f, ...) if(s2m->fdbg) fprintf(stderr, "%s:%i "f"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);
#define INF(f, ...) fprintf(stderr, "%s:%i "f"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);

static unsigned int sfmt[] = {	SND_PCM_FORMAT_S24_LE, \
								/*SND_PCM_FORMAT_S32_LE,*/ \
								SND_PCM_FORMAT_S24_3LE, \
								/*SND_PCM_FORMAT_DSD_U32_LE,*/ \
							};

typedef struct {
	snd_pcm_extplug_t ext;
	int fdbg;			// debug flag
	uint32_t lval;
	uint32_t rval;
	uint8_t tlval;
	uint8_t trval;
	uint8_t step;		// for debug
	snd_pcm_uframes_t buffer_size, period_size;
	snd_pcm_uframes_t buffer_time, period_time;
	uint32_t periods;
	uint8_t fhw;
} s2m_t;


static int s2m_init(snd_pcm_extplug_t *ext)
{
//	s2m_t *s2m = (s2m_t *)ext;

	return 0;
}

static int s2m_close(snd_pcm_extplug_t *ext)
{
//	s2m_t *s2m = (s2m_t *)ext;
	return 0;
}

static inline void *area_addr(const snd_pcm_channel_area_t *area,
			      snd_pcm_uframes_t offset)
{
	unsigned int bitofs = area->first + area->step * offset;
	return (char *) area->addr + bitofs / 8;
}

union uframe {
	uint64_t f64;
	uint32_t f32[2];
	uint16_t f16[4];
	uint8_t f8[8];
};

static snd_pcm_sframes_t
s2m_transfer(snd_pcm_extplug_t *ext,
	     const snd_pcm_channel_area_t *dst_areas,
	     snd_pcm_uframes_t dst_offset,
	     const snd_pcm_channel_area_t *src_areas,
	     snd_pcm_uframes_t src_offset,
	     snd_pcm_uframes_t size)
{
	s2m_t *s2m = (s2m_t *)ext;
	void *ipv = area_addr(src_areas, src_offset);
	uint8_t istep = 0;
	uint64_t *ip;
	uint64_t *op = area_addr(dst_areas, dst_offset);
	unsigned int count = size;
	union uframe is;
	union uframe os;
	uint32_t lval, rval;
	uint8_t tlval = s2m->tlval, trval = s2m->trval;

//	if( !(dst_areas->step == biic->cn * biic->wso * 8) || !(src_areas->step == biic->cn * biic->wsi * 8) ) {

	if( src_areas->step == 48 ) istep = 6;
	else if( src_areas->step == 64 ) istep = 8;

//	if( !(dst_areas->step == src_areas->step) || !(dst_areas->step == 64) ) {
	if( !istep || !(dst_areas->step == 64) ) {
		s2m->fdbg = 1;
		DBG( "Error in STEP, istep=%u    ostep=%u\n", src_areas->step, dst_areas->step );
//		DBG( "istep=%u ostep=%u ich=%u och=%u r=%u if=%u of=%u", 
//			src_areas->step, dst_areas->step, ext->channels, ext->slave_channels, ext->rate, ext->format, ext->slave_format );
//		my_hw_free( ext );
//		s2m_close( ext );
		return -EINVAL;
	}

//	memcpy(dst, src, count * 8);

	if( tlval ) lval = s2m->lval;
	if( trval ) rval = s2m->rval;

	do {

		ip = ipv; is.f64 = *ip;
	
		if( istep == 6 ) {
			is.f8[7] = 0;
			is.f8[6] = is.f8[5];
			is.f8[5] = is.f8[4];
			is.f8[4] = is.f8[3];
			is.f8[3] = 0;
		}

		os.f16[0] = is.f16[1]; os.f16[1] = is.f16[0];
		os.f16[2] = is.f16[3]; os.f16[3] = is.f16[2];

		if( tlval ) os.f32[0] = lval;
		if( trval ) os.f32[1] = rval;

		*op = os.f64;

		ipv += istep;
		op++;

	} while(count--);

	return size;
}

static const snd_pcm_extplug_callback_t s2m_callback = {
	.transfer = s2m_transfer,
	.init = s2m_init,
	.close = s2m_close,
};

#include "parm.h"

#define ARRAY_SIZE(ary)	(sizeof(ary)/sizeof(ary[0]))

SND_PCM_PLUGIN_DEFINE_FUNC(s2mono)
{
	snd_config_iterator_t i, next;
	s2m_t *s2m;
	snd_config_t *sconf = NULL;
	int err;

	s2m = calloc(1, sizeof(*s2m));
	if (!s2m)
		return -ENOMEM;

	s2m->step = 0;
	s2m->fdbg = 0;
	s2m->lval = 0;
	s2m->rval = 0;
	s2m->tlval = 0;
	s2m->trval = 0;
	s2m->buffer_size = 0; s2m->buffer_time = 0; s2m->periods = 0;
	s2m->period_size = 0; s2m->period_time = 0;
	s2m->fhw = 1;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0 || strcmp(id, "type") == 0 ||
		    strcmp(id, "hint") == 0)
			continue;
		if (strcmp(id, "slave") == 0) {
			sconf = n;
			continue;
		}

		err = get_bool_parm(n, id, "debug", &s2m->fdbg);
		if (err)
			goto ok;

		err = get_int_parm(n, id, "lval", &s2m->lval);
		if (err) {
			s2m->tlval = 1;
			fprintf( stderr, "lval=0x%x\n", s2m->lval);
			goto ok;
		}

		err = get_int_parm(n, id, "rval", &s2m->rval);
		if (err) {
			s2m->trval = 1;
			fprintf( stderr, "rval=0x%x\n", s2m->rval);
			goto ok;
		}

		SNDERR("Unknown field %s", id);
		err = -EINVAL;
	ok:
		if (err < 0) goto lerr;
	}

	if (!sconf) {
		SNDERR("No slave configuration for s2mono pcm");
		err = -EINVAL;
		goto lerr;
	}

	s2m->ext.version = SND_PCM_EXTPLUG_VERSION;
	s2m->ext.name = "2ch to 4ch Plugin";
	s2m->ext.callback = &s2m_callback;
	s2m->ext.private_data = s2m;

	err = snd_pcm_extplug_create(&s2m->ext, name, root, sconf,
				     stream, mode);
	if (err < 0) {
		goto lerr;
	}

	snd_pcm_extplug_set_param_link(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 0);
	snd_pcm_extplug_set_param_link(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, 0);

	snd_pcm_extplug_set_param(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 2);
	snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 4);
	snd_pcm_extplug_set_param_list( &s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, ARRAY_SIZE(sfmt), sfmt);
//	snd_pcm_extplug_set_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_S24);
	snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_S16);

	*pcmp = s2m->ext.pcm;
	return 0;

lerr:
	free(s2m);
	return err;

}

SND_PCM_PLUGIN_SYMBOL(s2mono);
