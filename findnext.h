Int_t genfuncs(TF1 *cf_length[],TF1 *cf_angle[]);
Int_t setnextstartpoint(Int_t k, Double_t xx[], Double_t yy[], TF1 *cf_length[], TF1 *cf_angle[]);
Int_t updatepoint(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[]); 
Float_t CalcCost(TH2F *hist, Int_t k, Int_t xbin, Int_t ybin, Double_t xx[], Double_t yy[],  TF1 *cf_length[], TF1 *cf_angle[]);
