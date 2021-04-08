int DH_generate_key(DH *dh)
{
    return dh->meth->generate_key(dh);
}