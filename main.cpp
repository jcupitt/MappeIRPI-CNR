#include <ctime>
#include <iomanip>
#include <ImageMagick-7/Magick++.h>
#include <gdal_priv.h>
#include <filesystem>

/**
 * Generate colored, upscaled, temporized map, with the Italian alert zones
 * and an Italy background, animated Gifs.
 */

using namespace std::string_literals;
namespace fs = std::filesystem;

static std::string DATE_FORMAT = "%Y%m%d_%H";
static std::string TEMP_PATH = "/home/giovanni/Desktop/TEMP/";
static std::string PREVISTE = "/cf_psm.tif";
static std::string BASE_PATH = "/home/giovanni/Desktop/dati";

/**
 * Converts UTC time string to a tm struct.
 * To change the folders name set the format below @line 26
 * for more information check this table @link http://www.cplusplus.com/reference/ctime/strftime/
 *
 * @param dateTime the string to convert
 */
tm toTime(std::stringstream dateTime) {
    struct tm tm{};
    dateTime >> std::get_time(&tm, "%Y%m%d_%H");
    return tm;
}

int main(int argc, char* argv[]) {

    char dirName[13];
    tm startDate = toTime(std::stringstream(argv[1]));
    tm endDate = toTime(std::stringstream(argv[2]));

    int diffHours = (int) std::difftime(mktime(&endDate), mktime(&startDate)) / 3600;

    GDALAllRegister();
    GDALDataset *originalDataset[diffHours];
    GDALDataset *newDataset[diffHours];
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

    for (int i = 0; i < diffHours; ++i) {
        strftime(dirName, 13, DATE_FORMAT.c_str(), &startDate);
        fs::create_directory();
        std::cout << "\n"s + dirName;
        originalDataset[0] = (GDALDataset *) GDALOpen((BASE_PATH + "/"s + dirName + PREVISTE).c_str(), GA_ReadOnly);
        newDataset[0] = driver->CreateCopy((TEMP_PATH + dirName + PREVISTE).c_str(), originalDataset[0], FALSE, nullptr,
                                           nullptr, nullptr);

    }

    for (int i = 0; i < diffHours; ++i) {
        GDALClose(newDataset[i]);
    }


    return 0;
}