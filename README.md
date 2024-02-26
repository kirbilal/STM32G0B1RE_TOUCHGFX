# STM32G0B1RE_TOUCHGFX

## STM32CubeMX Tarafında Yapılacaklar
Öncelikle STM32CubeIDE üzerinden proje açılır.
Kullanılacak TFT ekranın çalışabilmesi için gerekli olan SPI ayarının STM32CubeMX içerisinde yapılması gerekir.
Burada Transmit Only Master modu seçilir. Baud Rate 32 MBits/s olarak seçilir.

## TFT Ekran STM32CubeMX Konfigürasyonları


<div align="center">
  <img width="70%" height="70%" src="Documents/Images/DISPLAY_CubeMX_SPI_Configurations.png">
</div>
<br />

STM32CubeMX içerisinde TFT için kullanılan SPI1'in pin konfigürasyonları yapılmıştır. SPI pinlerinin Pull-down olarak seçilmesi gerekmektedir. Ayrıca maksimum çıkış hızı High seçilmiştir. 

TFT için<br>
DISPLAY_CSX   -> PB5 -> Pull-up -> Maksimum çıkış hızı -> Very High  <br>
DISPLAY_DCX   -> PB3 -> Pull-up -> Maksimum çıkış hızı -> Very High<br>
DISPLAY_TE    -> PA0 -> External Interrupt olarak ayarlanmıştır.<br>
DISPLAY_RESET -> PA1 -> Pull-up -> Maksimum çıkış hızı -> High<br>

<div align="center">
  <img width="70%" height="70%" src="Documents/Images/DISPLAY_CubeMX_GPIO_Configurations.png">
</div>
<br />

SPI kullanırken DMA kullandığımız için DMA ayarı yapılır. STM32 mikrodenetleyici kartından TFT'ye doğru tek taraflı iletim olduğu için sadece SPI1_TX için DMA kullanılır. Ayrıca burada Data Width seçeneğinin Half Word olarak seçilmesi gerekmektedir.<br /> SPI2_TX ve SPI2_RX için Data Width Byte olarak bırakılır.

<div align="center">
  <img width="70%" height="70%" src="Documents/Images/DISPLAY_DMA_Configurations.png">
</div>
<br />

## Nor Flash STM32CubeMX Konfigürasyonları

External Flash birimine ulaşmak için SPI2 kullanılmıştır. Baud Rate olarak 32 MBits/s seçilmiştir.

<div align="center">
  <img width="70%" height="70%" src="Documents/Images/Nor_Flash_CubeMX_Configurations.png">
</div>
<br />

SPI2 için GPIO konfigürasyonları yapılmıştır. SPI için kullanılan SCK, MISO, MOSI pinlerinin dirençleri Pull-down olarak ayarlanmıştır.<br>
Nor Flash için <br>
FLASH_CS -> PB9 -> Maksimum çıkış hızı -> Very High -> Fast mode : Enable<br>
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/Nor_Flash_CubeMX_GPIO_Configurations.png">
</div>
<br />

## CPT STM32CubeMX Konfigürasyonu

Kullanılan dokunmatik ekran için I2C2 kullanılmıştır. Hız olarak 400 Khz kullanılmıştır.

<div align="center">
  <img width="70%" height="70%" src="Documents/Images/CPT_I2C_Configurations.png">
</div>
<br />

CPT için pin konfigürasyonları yapılmıştır.
CPT_INT   -> PB7 External Interrupt olarak ayarlanmıştır.
CPT_RESET -> PA15 olarak ayarlanmıştır.
I2C için SDA ve SCL pinleri Pull-up olarak seçilmiştir.
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/CPT_GPIO_Configurations.png">
</div>
<br />

## TouchGFX 

Öncelikle X-CUBE-TOUCHGFX yazılım paketinin STM32CubeMX içerisinde yüklenmesi gerekir.<br>
Bunun için Help -> Manage embedded software packages -> STMicroelectronics kısmına girilir ve gerekli sürüm yüklenir.
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/TouchGFX_Package.png">
</div>
<br />

Paket yüklendikten sonra Middleware and Software Packs altında X-CUBE-TOUCHGFX'e tıklanır ve Graphics Application tiklenir.<br>
Ardından şekildeki gibi Please Enable CRC IP uyarısını gidermek için Computing -> CRC kısmından aktif hale getirilir.
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/TouchGFX_Enable_CRC.png">
</div>
<br />

TouchGFX Generator ayarları yapılır.
Burada ekran boyutu 240x320 olarak ayarlanmıştır. Partial Framebuffer Vsync ve External Data Reader seçeneği aktif hale getirilmiştir.
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/TouchGFX_Configurations.png">
</div>
<br />

Ardından STM32CubeIDE tarafına geçilir.
Proje dosyaları içerisinden TouchGFX klasörü altında target klasörü altındaki ApplicationTemplate.touchgfx.part dosyası açılır ve bizi TouchGFX Designer'a yönlendirir.
<br />
<div align="center">
  <img width="70%" height="70%" src="Documents/Images/touchgfx_blank_ui.png">
</div>
<br />
Burada Blank UI seçilir ve Import'a tıklanır. Açılan ekranda denemek için bir resim eklenir ve Generate Code(F4)'a tıklanır.
<br />
STM32CubeIDE uygulamasında refresh(F5)'e tıklandığında proje dosyaları içerisine Middlewares kısmında touchgfx dosyaları gelmiş olur.
