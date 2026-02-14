#include <stdio.h>
#include <string.h>
#include <ctype.h> // toupper() fonksiyonu için gerekli

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

    printf("Wordle Aviyonik - Hedef Kelime %ld harfli!\n", strlen(hedef));
    return 0;
}
