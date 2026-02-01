#include <stdio.h>

/*
 * Proje: YRT-YZL-2 Odev Cozumu
 * Soru 2: Konvolüsyon İşlemi
 * Hazirlayan: Emre Yilmaz
 * Tarih: 01.02.2026
 */

int main() {
    // 1. Verilen Matrislerin Tanimlanmasi

    // Goruntu Matrisi ]
    int G[4][4] = {
        {0, 1, 2, 0},
        {3, 4, 5, 2},
        {6, 7, 1, 1},
        {1, 2, 3, 4}
    };

    // Filtre Matrisi (F)
    int F[3][3] = {
        {0, 1, 0},
        {2, 3, 1},
        {0, 1, 2}
    };

    int M = 4; // Goruntu satir sayisi
    int N = 4; // Goruntu sutun sayisi
    int k = 3; // Filtre boyutu

    // 2. Filtre Toplaminin Hesaplanmasi
    int filtreToplami = 0;
    for(int i = 0; i < k; i++) {
        for(int j = 0; j < k; j++) {
            filtreToplami += F[i][j];
        }
    }
    printf("Filtre Toplami (Bolum Degeri): %d\n\n", filtreToplami);

    // 3. Konvolusyon İslemi
    printf("--- Sonuc Matrisi (R) ---\n");
    for(int i = 0; i < (M - k + 1); i++) {
        for(int j = 0; j < (N - k + 1); j++) {

            int toplam = 0;

            // Filtre matrisi icindeki çarpim işlemi
            for(int fi = 0; fi < k; fi++) {
                for(int fj = 0; fj < k; fj++) {
                    toplam += G[i + fi][j + fj] * F[fi][fj];
                }
            }

            // Hesaplanan toplami filtre ağırlığına böl
            int sonucPiksel = toplam / filtreToplami;

            printf("%d\t", sonucPiksel);
        }
        printf("\n");
    }

    return 0;
}
