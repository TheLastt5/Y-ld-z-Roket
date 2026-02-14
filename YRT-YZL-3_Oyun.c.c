#include <stdio.h>
#include <string.h>
#include <ctype.h> // toupper() fonksiyonu için gerekecek

void durumuGoster(char* tahmin, char* hedef, char* yanlisYerde, char* olmayan) {
    int uzunluk = strlen(hedef);
    printf("Cikti: ");
    for (int i = 0; i < uzunluk; i++) {
        if (tahmin[i] == hedef[i]) printf("%c ", hedef[i]);
        else printf("__ ");
    }
    printf("- Yeri yanlis: %s - Olmayan: %s\n", yanlisYerde, olmayan);
}

int main() {
    char hedef[] = "ROKET";
    char tahmin[20];
    char yanlisYerde[30] = "";
    char olmayanlar[30] = "";

    printf("Hedef Kelime %ld harfli!\n", strlen(hedef));

    while (1) {
        printf("Girdi: ");
        scanf("%s", tahmin);

        // KÜÇÜK HARFİ BÜYÜK HARFE ÇEVİRME BLOĞU
        for(int i = 0; tahmin[i]; i++){
            tahmin[i] = toupper(tahmin[i]);
        }

        if (strcmp(tahmin, hedef) == 0) {
            printf("Tebrikler! Kelime: %s\n", hedef);
            break;
        }

        for (int i = 0; i < strlen(tahmin); i++) {
            if (tahmin[i] == hedef[i]) {
                continue;
            } else if (strchr(hedef, tahmin[i])) {
                if (!strchr(yanlisYerde, tahmin[i])) {
                    int len = strlen(yanlisYerde);
                    yanlisYerde[len] = tahmin[i];
                    yanlisYerde[len+1] = ',';
                    yanlisYerde[len+2] = '\0';
                }
            } else {
                if (!strchr(olmayanlar, tahmin[i])) {
                    int len = strlen(olmayanlar);
                    olmayanlar[len] = tahmin[i];
                    olmayanlar[len+1] = ',';
                    olmayanlar[len+2] = '\0';
                }
            }
        }
        durumuGoster(tahmin, hedef, yanlisYerde, olmayanlar);
    }
    return 0;
}
