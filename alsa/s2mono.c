
#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>

#include <unistd.h>

#define DBG(f, ...) if(s2m->fdbg) fprintf(stderr, "%s:%i "f"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);
#define INF(f, ...) fprintf(stderr, "%s:%i "f"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);

#define hi "1"
#define lo "0"

#define gpio_request( pin ) \
{ fd = open("/sys/class/gpio/export", O_WRONLY);\
sprintf(buf, "%d", (pin) );\
write(fd, buf, strlen(buf));\
close(fd); }

#define gpio_free( pin ) \
{ fd = open("/sys/class/gpio/unexport", O_WRONLY);\
sprintf(buf, "%d", (pin) );\
write(fd, buf, strlen(buf));\
close(fd); }

#define gpio_set_value( pin, val) \
{ sprintf(buf, "/sys/class/gpio/gpio%d/value", (pin) );\
fd = open(buf, O_WRONLY);\
write(fd, (val) , 1);\
close(fd); }

#define gpio_direction_output( pin, val) \
{ gpio_request( (pin) );\
sprintf(buf, "/sys/class/gpio/gpio%d/direction", (pin) );\
fd = open(buf, O_WRONLY);\
write(fd, "out", 3);\
close(fd);\
gpio_set_value( (pin) , (val) );}

int fd;
char buf[64];

static unsigned int sfmt[] = {	SND_PCM_FORMAT_S16_LE, \
								SND_PCM_FORMAT_S24_LE, \
								SND_PCM_FORMAT_S24_3LE, \
								SND_PCM_FORMAT_DSD_U32_LE, \
							};
static unsigned int ofmt[] = {	SND_PCM_FORMAT_S24_LE, \
								/*SND_PCM_FORMAT_S32_LE,*/ \
								/*SND_PCM_FORMAT_S24_3LE,*/ \
								SND_PCM_FORMAT_DSD_U32_LE, \
							};

typedef struct {
	snd_pcm_extplug_t ext;
	int fdbg;			// debug flag
	int f243;			// S24_3LE 
	int s2mono;			// 0=copy s16-s24
	uint32_t lval;
	uint32_t rval;
	uint8_t tlval;
	uint8_t trval;
	uint8_t step;		// for debug
				// PIN numbers = (letter-1)*32+pinnum for NanoPi
	int dsd;		// 0 - PCM; 1 - DSD
} s2m_t;


static int s2m_init(snd_pcm_extplug_t *ext)
{
//	s2m_t *s2m = (s2m_t *)ext;
	
	fprintf( stderr, "Init" );

	return 0;
}

static int s2m_close(snd_pcm_extplug_t *ext)
{
	s2m_t *s2m = (s2m_t *)ext;
	if( s2m->dsd )
		gpio_free( s2m->dsd );
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
	void *opv = area_addr(dst_areas, dst_offset);
	uint8_t istep = 0, ostep = 0;
	uint64_t *ip;
	uint64_t *op;
	unsigned int count = size;
	union uframe is;
	union uframe os;
	uint32_t lval, rval;
	uint8_t tlval = s2m->tlval, trval = s2m->trval;
	int s2mono = s2m->s2mono;

	if( src_areas->step == 32 ) istep = 4;
	else if( src_areas->step == 48 ) istep = 6;
	else if( src_areas->step == 64 ) istep = 8;

	if( dst_areas->step == 32 ) ostep = 4;
	else if( dst_areas->step == 48 ) ostep = 6;
	else if( dst_areas->step == 64 ) ostep = 8;

	if( ( !istep || !ostep ) ||
		( istep==4 && ostep!=4 ) ||		// s16 -> s16
		( istep==6 && ostep!=8 ) ||		// s24_3 -> s24
		( istep==8 && ostep!=8 ) )		// s24, dsd32
	{
		//s2m->fdbg = 1;
		INF( "Error in STEP, istep=%u    ostep=%u\n", src_areas->step, dst_areas->step );
//		DBG( "istep=%u ostep=%u ich=%u och=%u r=%u if=%u of=%u", 
//			src_areas->step, dst_areas->step, ext->channels, ext->slave_channels, ext->rate, ext->format, ext->slave_format );
//		my_hw_free( ext );
//		s2m_close( ext );
		return -EINVAL;
	}


		// s16 copy
	if( istep == 4 ) {
		memcpy(opv, ipv, count * 4);
		return size;
	}

		// s24 copy
	if( !s2mono && ext->format == SND_PCM_FORMAT_S24_LE ) {
		memcpy(opv, ipv, count * 8);
		return size;
	}

	if( ext->format == SND_PCM_FORMAT_DSD_U32_LE ) s2mono = 1;

	if( tlval ) lval = s2m->lval;
	if( trval ) rval = s2m->rval;

	op = opv;
	do {

		ip = ipv; is.f64 = *ip;
	
		if( istep == 6 ) {
			is.f8[7] = 0;
			is.f8[6] = is.f8[5];
			is.f8[5] = is.f8[4];
			is.f8[4] = is.f8[3];
			is.f8[3] = 0;
		}

		if( tlval ) is.f32[0] = lval;
		if( trval ) is.f32[1] = rval;

		if( s2mono ) {				// s24 dsd
			os.f16[0] = is.f16[1]; os.f16[1] = is.f16[0];
			os.f16[2] = is.f16[3]; os.f16[3] = is.f16[2];
		} else 
			os.f64 = is.f64;		// copy s24_3 -> s24

		*op = os.f64;

		ipv += istep;
		op++;

	} while(--count);

	return size;
}


static int s2m_hw(snd_pcm_extplug_t *ext, snd_pcm_hw_params_t *params)
{
	s2m_t *s2m = (s2m_t *)ext;
//	snd_pcm_t *pcm = s2m->ext.pcm;
//	int ret, hw_set = 0;

	if( s2m->dsd ) {
		if( ext->format == SND_PCM_FORMAT_DSD_U32_LE ) {
			gpio_set_value( s2m->dsd, hi);
		} else {
			gpio_set_value( s2m->dsd, lo);
		}
	}

	return 0;
//	INF( "rate = %u\n", ext->rate );
//	fprintf( stderr, "0 ich=%u och=%u r=%u if=%u of=%u\n", 
//			ext->channels, ext->slave_channels, ext->rate, ext->format, ext->slave_format );
}

static const snd_pcm_extplug_callback_t s2m_callback = {
	.transfer = s2m_transfer,
	.init = s2m_init,
	.close = s2m_close,
	.hw_params = s2m_hw,
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
	s2m->s2mono = 0;
	s2m->f243 = 0;
	s2m->lval = 0;
	s2m->rval = 0;
	s2m->tlval = 0;
	s2m->trval = 0;
	s2m->dsd = 0;

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
		if (err) {
			fprintf( stderr, "debug=0x%d\n", s2m->fdbg);
			goto ok;
		}

		err = get_bool_parm(n, id, "s2mono", &s2m->s2mono);
		if (err) {
			fprintf( stderr, "s2mono=0x%d\n", s2m->s2mono);
			goto ok;
		}

		err = get_bool_parm(n, id, "f243", &s2m->f243);
		if (err) {
			fprintf( stderr, "f243=0x%d\n", s2m->f243);
			goto ok;
		}

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

		err = get_int_parm(n, id, "dsd", &s2m->dsd);
		if (err) {
			fprintf( stderr, "dsd=%d\n", s2m->dsd);
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

	// !!! !!! !!! required
	snd_pcm_extplug_set_param_link(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 0);
	snd_pcm_extplug_set_param(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 2);
	snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_CHANNELS, 2);
	// !!! !!! !!!

	if( s2m->f243 ) {
		snd_pcm_extplug_set_param_link(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, 0);
		snd_pcm_extplug_set_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_S24_3LE);
		snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_S24);
	} else {
		snd_pcm_extplug_set_param_link(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, 1);
		snd_pcm_extplug_set_param_list( &s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, ARRAY_SIZE(sfmt), sfmt);
	}

//	snd_pcm_extplug_set_param_list( &s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, ARRAY_SIZE(sfmt), sfmt);
//	snd_pcm_extplug_set_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_DSD_U32_LE);
//	snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_DSD_U32_LE);
//	snd_pcm_extplug_set_slave_param(&s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, SND_PCM_FORMAT_S16);
//	snd_pcm_extplug_set_slave_param_list( &s2m->ext, SND_PCM_EXTPLUG_HW_FORMAT, ARRAY_SIZE(ofmt), ofmt);

	*pcmp = s2m->ext.pcm;
	return 0;

lerr:
	free(s2m);
	return err;

}

SND_PCM_PLUGIN_SYMBOL(s2mono);
