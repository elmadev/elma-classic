
void alaphelp(void); // O figyeli ki a tool

void t_move_nyomva(int x, int y);
void t_move_nyomva_jobb(int x, int y);
void t_move_esc(void);
void t_move_mmove(int x, int y);
// Kurzor negyzette valtozik:
// void t_move_space( void );
// extern int Kurzornegyzet;

extern int Egypont;
void t_create_vert_nyomva(int x, int y);
void t_create_vert_esc(void);
void t_create_vert_space(void);
void t_create_vert_enter(void);
void t_create_vert_mmove(int x, int y);

void t_delete_vert_nyomva(int x, int y);
void t_delete_poly_nyomva(int x, int y);

// Legkozelebbi create food 0
extern int Food_kajatipus, Food_foodsorszam;
void t_create_food_nyomva_jobb(void);
// Food es killer is egyben:
void t_create_kerek_nyomva(int x, int y, int food);
void t_delete_kerek_nyomva(int x, int y);

void t_create_sprite_nyomva(int x, int y);
void t_create_sprite_nyomva_jobb(void);
void t_delete_sprite_nyomva(int x, int y);

extern int Zoomfogva; // Ha ez igaz, akkor nem szabad gombokat nyomkodni
void t_zoomin_nyomva(int x, int y);
void t_zoomin_esc(void);
void t_zoomin_mmove(int x, int y);
