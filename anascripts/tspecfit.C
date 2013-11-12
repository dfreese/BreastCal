Double_t DoubleGaussConstExpo(Double_t *x, Double_t *par)

/* The signal function: a gaussian */
{
        /*~~~~~~~~~~~~~~~~*/
        Double_t        arg = 0;
        Double_t        arg2 = 0;
        /*~~~~~~~~~~~~~~~~*/
        if(par[5]) { arg = (x[0] - par[1]) / par[2]; arg2 = (x[0] - par[4]) / par[2];}
                
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        Double_t        sig = par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] * TMath::Exp(-0.5 * arg2 *arg2 ) + par[5] + TMath::Exp(par[6]+par[7]*x[0]*x[0]);
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        return sig;
}
