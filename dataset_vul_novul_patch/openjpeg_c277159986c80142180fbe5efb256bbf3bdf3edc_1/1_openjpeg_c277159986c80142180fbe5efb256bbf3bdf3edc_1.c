static opj_bool pi_next_pcrl(opj_pi_iterator_t * pi)
{
    opj_pi_comp_t *comp = NULL;
    opj_pi_resolution_t *res = NULL;
    long index = 0;

    if (!pi->first) {
        comp = &pi->comps[pi->compno];
        goto LABEL_SKIP;
    } else {
        int compno, resno;
        pi->first = 0;
        pi->dx = 0;
        pi->dy = 0;
        for (compno = 0; compno < pi->numcomps; compno++) {
            comp = &pi->comps[compno];
            for (resno = 0; resno < comp->numresolutions; resno++) {
                int dx, dy;
                res = &comp->resolutions[resno];
                dx = comp->dx * (1 << (res->pdx + comp->numresolutions - 1 - resno));
                dy = comp->dy * (1 << (res->pdy + comp->numresolutions - 1 - resno));
                pi->dx = !pi->dx ? dx : int_min(pi->dx, dx);
                pi->dy = !pi->dy ? dy : int_min(pi->dy, dy);
            }
        }
    }
    if (!pi->tp_on) {
        pi->poc.ty0 = pi->ty0;
        pi->poc.tx0 = pi->tx0;
        pi->poc.ty1 = pi->ty1;
        pi->poc.tx1 = pi->tx1;
    }
    for (pi->y = pi->poc.ty0; pi->y < pi->poc.ty1;
            pi->y += pi->dy - (pi->y % pi->dy)) {
        for (pi->x = pi->poc.tx0; pi->x < pi->poc.tx1;
                pi->x += pi->dx - (pi->x % pi->dx)) {
            for (pi->compno = pi->poc.compno0; pi->compno < pi->poc.compno1; pi->compno++) {
                comp = &pi->comps[pi->compno];
                for (pi->resno = pi->poc.resno0;
                        pi->resno < int_min(pi->poc.resno1, comp->numresolutions); pi->resno++) {
                    int levelno;
                    int trx0, try0;
                    int trx1, try1;
                    int rpx, rpy;
                    int prci, prcj;
                    res = &comp->resolutions[pi->resno];
                    levelno = comp->numresolutions - 1 - pi->resno;
                    trx0 = int_ceildiv(pi->tx0, comp->dx << levelno);
                    try0 = int_ceildiv(pi->ty0, comp->dy << levelno);
                    trx1 = int_ceildiv(pi->tx1, comp->dx << levelno);
                    try1 = int_ceildiv(pi->ty1, comp->dy << levelno);
                    rpx = res->pdx + levelno;
                    rpy = res->pdy + levelno;

                    /* To avoid divisions by zero / undefined behaviour on shift */
                    if (rpx >= 31 || ((comp->dx << rpx) >> rpx) != comp->dx ||
                            rpy >= 31 || ((comp->dy << rpy) >> rpy) != comp->dy) {
                        continue;
                    }

                    if (!((pi->y % (comp->dy << rpy) == 0) || ((pi->y == pi->ty0) &&
                            ((try0 << levelno) % (1 << rpy))))) {
                        continue;
                    }
                    if (!((pi->x % (comp->dx << rpx) == 0) || ((pi->x == pi->tx0) &&
                            ((trx0 << levelno) % (1 << rpx))))) {
                        continue;
                    }

                    if ((res->pw == 0) || (res->ph == 0)) {
                        continue;
                    }

                    if ((trx0 == trx1) || (try0 == try1)) {
                        continue;
                    }

                    prci = int_floordivpow2(int_ceildiv(pi->x, comp->dx << levelno), res->pdx)
                           - int_floordivpow2(trx0, res->pdx);
                    prcj = int_floordivpow2(int_ceildiv(pi->y, comp->dy << levelno), res->pdy)
                           - int_floordivpow2(try0, res->pdy);
                    pi->precno = prci + prcj * res->pw;
                     for (pi->layno = pi->poc.layno0; pi->layno < pi->poc.layno1; pi->layno++) {
                         index = pi->layno * pi->step_l + pi->resno * pi->step_r + pi->compno *
                                 pi->step_c + pi->precno * pi->step_p;
                         if (!pi->include[index]) {
                             pi->include[index] = 1;
                             return OPJ_TRUE;
                        }
LABEL_SKIP:
                        ;
                    }
                }
            }
        }
    }

    return OPJ_FALSE;
}
