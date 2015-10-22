
class Cluster
{
public:
    float newmeanx,newmeany;
    int newpoints;
    double wcss;
    float curmeanx,curmeany;
    int curpoints;
    void Add (float,float,int);
    void NewIteration(void);
    float Distance(float,float);
    void Init(float,float);
    void Print(void) ;
    Cluster();
};
