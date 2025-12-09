
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

    // Terbeli kereseshez:
    pvonalnode* tertomb;
    int xdim, ydim;
    double cellameret;
    vekt2 origo;
    vonalnode* nextnode; // getnext ez alapjan dolgozik!

    void beindexelvonalat(vonal* pv, double maxtav);
    void bekot(int cellax, int cellay, vonal* pv);

    // Terbeli kereseshez nodkezeles:
    soknodecsomag* pelsocsomag;
    int csomagbanbetelt;
    vonalnode* getnewnode(void);
    void deleteallnodes(void);

  public:
    szakaszok(topol* ptop);
    ~szakaszok(void);

    // Terbeli fv.-ek:
    // rendez-t csak egyszer szabad hivni!
    void rendez(double maxtav);
    void felsorolasreset(vekt2 r);
    vonal* getnext(void);

    // Osszes szakaszon vegigmennek:
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
