#ifndef T3F_ACHIEVEMENTS_H
#define T3F_ACHIEVEMENTS_H

typedef struct
{

  char * name;
  char * description;
  int steps;
  int step;

} T3F_ACHIEVEMENT_ENTRY;

typedef struct
{

  T3F_ACHIEVEMENT_ENTRY * entry;
  int entries;

} T3F_ACHIEVEMENTS_LIST;

T3F_ACHIEVEMENTS_LIST * t3f_create_achievements_list(int entries);
void t3f_destroy_achievements_list(T3F_ACHIEVEMENTS_LIST * achievements_list);
bool t3f_set_achievement_details(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, const char * name, const char * description, int steps);
void t3f_update_achievement_progress(T3F_ACHIEVEMENTS_LIST * achievements_list, int entry, int step);
void t3f_save_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);
void t3f_load_achievements_data(T3F_ACHIEVEMENTS_LIST * achievements_list, ALLEGRO_CONFIG * config, const char * section);

#endif
