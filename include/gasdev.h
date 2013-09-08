/*************************************************
 This is noise subprograms
**************************************************/

int idumm=-466;
float
gasdev (int *idum)
{
  static int iset = 0;
  static float gset;
  float fac, r, v1, v2;
  float ran3 (int *);

  if (iset == 0)
  {
    do
    {
      v1 = 2.0 * ran3 (idum) - 1.0;
      v2 = 2.0 * ran3 (idum) - 1.0;
      r = v1 * v1 + v2 * v2; 
    }
    while (r >= 1.0);
    fac = sqrt (-2.0 * log (r) / r);
    gset = v1 * fac;
    iset = 1;
    return v2 * fac;
  }
  else
  {
    iset = 0;
    return gset;
  }
}
#define MBIG 1000000000L
#define MSEED 161803398L
#define MZ 0
#define FAC (1.0/MBIG)
float ran3 (int *idum)
{
  static int inext, inextp;
  static long ma[57];
  static int iff = 0;
  long mj,mk;
  int i, ii, k;

  if (*idum < 0 || iff == 0)
  {
    iff = 1;
    mj = MSEED - (*idum < 0 ? -*idum : *idum);
    mj %= MBIG;
    ma[55] = mj;
    mk = 1;
    for (i = 1; i <= 54; i++)
    {
      ii = (21 * i) % 55;
      ma[ii] = mk;
      mk = mj - mk;
      if (mk < MZ)
	mk += MBIG;
      mj = ma[ii];
    }
    for (k = 1; k <= 4; k++)
      for (i = 1; i <= 55; i++)
      {
	ma[i] -= ma[1 + (i + 30) % 55];
	if (ma[i] < MZ)
	  ma[i] += MBIG;
      }
    inext = 0;
    inextp = 31;
    *idum = 1;
  }
  if (++inext == 56)
    inext = 1;
  if (++inextp == 56)
    inextp = 1;
  mj = ma[inext] - ma[inextp];
  if (mj < MZ)
    mj += MBIG;
  ma[inext] = mj;
  return mj * FAC;
}




