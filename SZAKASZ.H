
struct vonal;

struct vonalnode {
    vonal* pvonal;
    vonalnode* pnext;
};

typedef vonalnode* pvonalnode;

#define CSOMAGBANNODESZAM (500)
struct soknodecsomag {
    vonalnode nodetomb[CSOMAGBANNODESZAM];
    soknodecsomag* nextcsomag;
};

class topol;

class szakaszok {
    vonal* tomb;
    int maxszam;
    int szam;
    int szakfuto;

    // for 3d search:
    pvonalnode* tertomb;
    int xdim, ydim;
    double cellameret;
    vekt2 origo;
    vonalnode* nextnode; // this is the base for getnext

    void beindexelvonalat(vonal* pv, double maxtav);
    void bekot(int cellax, int cellay, vonal* pv);

    // node management for 3d search
    soknodecsomag* pelsocsomag;
    int csomagbanbetelt;
    vonalnode* getnewnode(void);
    void deleteallnodes(void);

  public:
    szakaszok(topol* ptop);
    ~szakaszok(void);

    // 3d functions
    // rendez should only be called once
    void rendez(double maxtav);
    void felsorolasreset(vekt2 r);
    vonal* getnext(void);

    // iterate all sections
    void felsorolasresetszak(void);
    vonal* getnextszak(void);

    vonal* getptrszakasz(int sorszam);
    int getsorszam(vonal* pvonal);
};

struct vonal {
    vekt2 r, v, egyseg;
    double hossz;
};

extern szakaszok* Pszak;
