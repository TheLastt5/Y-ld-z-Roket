#include <stdio.h>

/*
 * Proje: YRT-YZL-2 Odev Cozumu
 * Soru 1: Matris Dondurme
 * Hazirlayan: Emre Yilmaz
 * Tarih: 01.02.2026
 */

enum GRange {
    AFS_2G = 0, // Binary: 00
    AFS_4G = 1, // Binary: 01
    AFS_8G = 2, // Binary: 10 (Bunu kullanıcaz)
    AFS_16G = 3 // Binary: 11
};

enum Bandwidth {
    HIZ_7_81Hz = 0,
    HIZ_62_5Hz = 3,
    HIZ_500Hz = 6, // Binary: 110 (Bunu kullanıcaz)
    HIZ_1000Hz = 7
};

enum OperationMode {
    NORMAL = 0,    // Binary: 000 (Bunu kullanıcaz)
    SUSPEND = 1,
    LOW_POWER = 2,
    STANDBY = 3
};

struct SensorYapisi {
    int g_range;       // G Aralığı
    int bandwidth;     // Hız
    int mode;          // Çalışma Modu
    unsigned char AccConfigValue; // Sonuç (8 bitlik sayı)
};

int main() {
    // Sensörümüzü oluşturuyoruz
    struct SensorYapisi benimSensorum;

    // 3. ADIM: Değerleri seçiyoruz
    benimSensorum.g_range = AFS_8G;    // Değeri 2
    benimSensorum.bandwidth = HIZ_500Hz; // Değeri 6
    benimSensorum.mode = NORMAL;       // Değeri 0

    // 4. ADIM: Bitleri kaydırmaca

    benimSensorum.AccConfigValue = (benimSensorum.mode << 5) |
                                   (benimSensorum.bandwidth << 2) |
                                   (benimSensorum.g_range);

    // 5. ADIM: Sonuçları ekrana basma
    printf("//Output\n");
    printf("AccConfigValue (Decimal): %d\n", benimSensorum.AccConfigValue);
    printf("AccConfigValue (Hexadecimal): 0x%x\n", benimSensorum.AccConfigValue);

    // Binary yazdırmak için basit bir döngü
    printf("AccConfigValue (Binary): 0b");

    // 7. bitten 0. bite kadar tek tek kontrol edip yazdırıyoruz
    for(int i = 7; i >= 0; i--) {
        int bit = (benimSensorum.AccConfigValue >> i) & 1;
        printf("%d", bit);
    }
    printf("\n");

    return 0;
}
