#include <stdio.h>

/*
 * Proje: YRT-YZL-2 Odev Cozumu
 * Soru 1: Matris Dondurme
 * Hazirlayan: Emre Yilmaz
 * Tarih: 01.02.2026
 */

int main() {
    int matris[100][100];
    int yeniMatris[100][100];

    int d;
    int secim;
    int sayac = 1;

    // 1. Kullanicidan boyutu al
    printf("Matris boyutunu giriniz: ");
    scanf("%d", &d); // N yerine d

    // 2. Matrisi 1'den baslayarak doldur
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < d; j++) {
            matris[i][j] = sayac;
            sayac++;
        }
    }

    printf("\n--- Orijinal Matris ---\n");
    for(int i = 0; i < d; i++) {
        for(int j = 0; j < d; j++) {
            printf("%d\t", matris[i][j]);
        }
        printf("\n");
    }

    // 4. Kullaniciya ne yapmak istedigini sor
    printf("\nIslem Seciniz (1: Saga Dondur, 2: Sola Dondur): ");
    scanf("%d", &secim);

    // 5. Dondurme Islemi
    if(secim == 1) {
        for(int i = 0; i < d; i++) {
            for(int j = 0; j < d; j++) {
                yeniMatris[j][d - 1 - i] = matris[i][j];
            }
        }
        printf("\n-- Saga Donmus Hali --\n");
    }
    else if(secim == 2) {
        for(int i = 0; i < d; i++) {
            for(int j = 0; j < d; j++) {
                yeniMatris[d - 1 - j][i] = matris[i][j];
            }
        }
        printf("\n-- Sola Donmus Hali --\n");
    }
    else {
        printf("Hatali secim yaptiniz!\n");
        return 0;
    }

    // 6. Sonucu Ekrana Yazdir
    for(int i = 0; i < d; i++) { // N yerine d
        for(int j = 0; j < d; j++) { // N yerine d
            printf("%d\t", yeniMatris[i][j]);
        }
        printf("\n");
    }

    return 0;
}
