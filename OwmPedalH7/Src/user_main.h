#ifndef USER_MAIN_H
#define USER_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void mainInit();

void mainLoop();

#ifdef __cplusplus
}
#endif

void touchProcess(uint16_t x, uint16_t y, uint8_t a);
void loadData();
void saveData();

#endif // USER_MAIN_H
