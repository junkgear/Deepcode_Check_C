int DH_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    return dh->meth->compute_key(key, pub_key, dh);
}