static int get_bool_parm(snd_config_t *n, const char *id, const char *str,
			 int *val_ret)
{
	int val;
	if (strcmp(id, str))
		return 0;

	val = snd_config_get_bool(n);
	if (val < 0) {
		SNDERR("Invalid value for %s", id);
		return val;
	}
	*val_ret = val;
	return 1;
}

static int get_int_parm(snd_config_t *n, const char *id, const char *str,
			int *val_ret)
{
	long val;
	int err;

	if (strcmp(id, str))
		return 0;
	err = snd_config_get_integer(n, &val);
	if (err < 0) {
		SNDERR("Invalid value for %s parameter", id);
		return err;
	}
	*val_ret = val;
	return 1;
}

static int get_int64_parm(snd_config_t *n, const char *id, const char *str,
			uint64_t *val_ret)
{
	long long val;
	int err;

	if (strcmp(id, str))
		return 0;
	err = snd_config_get_integer64(n, &val);
	if (err < 0) {
		SNDERR("Invalid value for %s parameter", id);
		return err;
	}
	*val_ret = val;
	return 1;
}

static int get_float_parm(snd_config_t *n, const char *id, const char *str,
			  float *val_ret)
{
	double val;
	int err;

	if (strcmp(id, str))
		return 0;
	err = snd_config_get_ireal(n, &val);
	if (err < 0) {
		SNDERR("Invalid value for %s", id);
		return err;
	}
	*val_ret = val;
	return 1;
}

