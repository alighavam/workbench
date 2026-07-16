/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#include "CaretLogger.h"
#include "DataFileException.h"
#include "GiftiLabel.h"
#include "GiftiMetaData.h"
#include "Palette.h"
#include "PaletteColorMapping.h"
#include "PaletteFile.h"
#include "PaletteScalarAndColor.h"

#include <limits>

using namespace caret;

/**
 * Constructor.
 *
 */
PaletteFile::PaletteFile()
: CaretDataFile(DataFileTypeEnum::PALETTE)
{
    this->metadata = new GiftiMetaData();
    this->initializeMembersPaletteFile();
    this->addDefaultPalettes();
    this->clearModified();
}

/**
 * Destructor
 */
PaletteFile::~PaletteFile()
{
    this->clearAll();
    delete this->metadata;
}

void
PaletteFile::initializeMembersPaletteFile()
{
}

/**
 * Get the label table used for color storage.
 * @return  LabelTable used for color storage.
 *
 */
GiftiLabelTable*
PaletteFile::getLabelTable()
{
    return &this->labelTable;
}

/**
 * Clear everything.
 */
void 
PaletteFile::clearAll()
{
    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        delete this->palettes[i];
    }
    this->palettes.clear();
    this->labelTable.clear();   
    this->metadata->clear();
}

/**
 * Clear the file but add default palettes.
 */
void
PaletteFile::clear()
{
    this->clearAll();
    this->addDefaultPalettes();
}

/**
 * Add a scalar and RGB INTEGER color to a palette.  Palette MUST have a name.
 * @param palette
 *    Palette that receives scalar and color
 * @param scalar
 *    The scalar value
 * @param red
 *    Red component [0, 255]
 * @param green
 *    Green component [0, 255]
 * @param blue
 *    Blue component [0, 255]
 */
void
PaletteFile::addPaletteScalarAndColor(Palette& palette,
                                      const float scalar,
                                      const int32_t red,
                                      const int32_t green,
                                      const int32_t blue)
{
    AString name(palette.getName());
    if (name.isEmpty()) {
        CaretAssertMessage(0, "Palette is missing name.  Palette will be invalid.");
        return;
    }
    
    /*
     * Name of color is name of palette followed by index of color
     */
    name.append("_" + AString::number(palette.getNumberOfScalarsAndColors()));
    
#ifndef NDEBUG
    if (this->labelTable.getLabel(name) != NULL) {
        CaretAssertMessage(0, ("Generation of color name failed.  Color \""
                               + name
                               + "\" already exists."));
    }
#endif
    
    this->addColor(name, red, green, blue);
    
    palette.addScalarAndColor(scalar, name);
}

/**
 * Add a scalar and RGB FLOAT color to a palette.  Palette MUST have a name.
 * @param palette
 *    Palette that receives scalar and color
 * @param scalar
 *    The scalar value
 * @param red
 *    Red component [0, 1]
 * @param green
 *    Green component [0, 1]
 * @param blue
 *    Blue component [0, 1]
 */
void
PaletteFile::addPaletteScalarAndColorFloat(Palette& palette,
                                           const float scalar,
                                           const float red,
                                           const float green,
                                           const float blue)
{
    addPaletteScalarAndColor(palette,
                             scalar,
                             red * 255.0,
                             green * 255.0,
                             blue * 255.0);
}

/**
 * Add a palette color.
 * 
 * @param pc - color to add.
 *
 */
void
PaletteFile::addColor(const GiftiLabel& pc)
{
    this->labelTable.addLabel(&pc);
}

/**
 * Add a palette color.
 * 
 * @param name  - name of color.
 * @param red   - red component.
 * @param green - red component.
 * @param blue  - red component.
 *
 */
void
PaletteFile::addColor(
                   const AString& name,
                   const int32_t red,
                   const int32_t green,
                   const int32_t blue)
{
    this->labelTable.addLabel(name, red, green, blue);
}

/**
 * Add a palette color.
 * 
 * @param name - Name of color.
 * @param rgb  - RGB components of color.
 *
 */
void
PaletteFile::addColor(
                   const AString& name,
                   const int32_t rgb[])
{
    this->addColor(name, rgb[0], rgb[1], rgb[2]);
}

/**
 * Get a color via its index.
 * 
 * @param index - index of color.
 * @return  Reference to color at index or the default color 
 *          if the index is invalid.
 *
 */
const GiftiLabel*
PaletteFile::getColor(const int32_t indx) const
{
    return this->labelTable.getLabel(indx);
}

/**
 * Get a color via its index.
 * 
 * @param colorName - Name of color.
 * @return  Reference to color with name or the default color 
 *          if the name does not match any colors.
 *
 */
const GiftiLabel*
PaletteFile::getColorByName(const AString& colorName) const
{
    const GiftiLabel* gl = this->labelTable.getLabel(colorName);
    return gl;
}

/**
 * Get index for a color.
 * 
 * @param colorName - Name of color.
 * @return  Index to color or -1 if not found. 
 *
 */
int32_t
PaletteFile::getColorIndex(const AString& colorName) const
{
    return this->labelTable.getLabelKeyFromName(colorName);
}

/**
 * Get the number of palettes.
 * 
 * @return The number of palettes.
 *
 */
int32_t
PaletteFile::getNumberOfPalettes() const
{
    return this->palettes.size();
}

/**
 * Add a palette.
 * 
 * @param p - palette to add.
 *
 */
void
PaletteFile::addPalette(const Palette& p)
{
    Palette* pal = new Palette(p);
    this->assignColorsToPalette(*pal);
    this->palettes.push_back(pal);
    this->setModified();
}

/**
 * Get a palette.
 * 
 * @param index - index of palette.
 * @return  Reference to palette or null if invalid index.
 *
 */
Palette*
PaletteFile::getPalette(const int32_t indx) const
{
    return this->palettes[indx];
}

/**
 * Find a palette by the specified name.
 * 
 * @param name  Name of palette to search for.
 * @return  Reference to palette with name or null if not found.
 *
 */
Palette*
PaletteFile::getPaletteByName(const AString& name) const
{
    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        if (this->palettes[i]->getName() == name) {
            return this->palettes[i];
        }
    }
    return NULL;
}

/**
 * Remove a palette.
 * 
 * @param index - index of palette to remove.
 *
 */
void
PaletteFile::removePalette(const int32_t indx)
{
    this->palettes.erase(this->palettes.begin() + indx);
    this->setModified();
}

/**
 * Is this file empty?
 * 
 * @return true if the file is empty, else false.
 *
 */
bool
PaletteFile::isEmpty() const
{
    return this->palettes.empty();
}

/**
 * String description of this class. 
 */
AString
PaletteFile::toString() const
{
    AString s;
    
    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        s += (this->palettes[i]->toString() + "\n");
    }
    
    return s;
}

/**
 * Is this palette modified?
 * @return
 *   true if modified, else false.
 */
bool 
PaletteFile::isModified() const
{
    if (DataFile::isModified()) {
        return true;
    }
    if (this->labelTable.isModified()) {
        return true;
    }
    
    const int64_t numberOfPalettes = this->getNumberOfPalettes();
    for (int i = 0; i < numberOfPalettes; i++) {
        if (this->palettes[i]->isModified()) {
            return true;
        }
    }
    return false;
}

/**
 * Set this object as not modified.  Object should also
 * clear the modification status of its children.
 *
 */
void
PaletteFile::clearModified()
{
    DataFile::clearModified();
    
    const int64_t numberOfPalettes = this->getNumberOfPalettes();
    for (int i = 0; i < numberOfPalettes; i++) {
        this->palettes[i]->clearModified();
    }
    
    this->labelTable.clearModified();
}

/**
 * Assign colors to the palette.
 * @param
 *    p Palette to which colors are assigned.
 */
void 
PaletteFile::assignColorsToPalette(Palette& p)
{
    int64_t numberOfScalars = p.getNumberOfScalarsAndColors();
    for (int64_t i = 0; i < numberOfScalars; i++) {
        PaletteScalarAndColor* psac = p.getScalarAndColor(i);
        const AString& colorName = psac->getColorName();
        const GiftiLabel* gl = this->getColorByName(colorName);
        if (gl != NULL) {
            float rgba[4];
            gl->getColor(rgba);
            psac->setColor(rgba);
        } else {
            CaretLogSevere(("Missing color \""
                            + colorName
                            + "\" in palette \""
                            + p.getName()
                            + "\""));
        }
    }
}

/**
 * Read the data file.
 *
 * @param filename
 *    Name of the data file.
 * @throws DataFileException
 *    If the file was not successfully read.
 */
void 
PaletteFile::readFile(const AString& filename)
{
    clear();
//    checkFileReadability(filename);
    
    throw DataFileException(filename,
                            "Reading of PaletteFile not implemented.");
}

/**
 * Write the data file.
 *
 * @param filename
 *    Name of the data file.
 * @throws DataFileException
 *    If the file was not successfully written.
 */
void 
PaletteFile::writeFile(const AString& filename)
{
//    checkFileWritability(filename);
    
    throw DataFileException(filename,
                            "Reading of PaletteFile not implemented.");
}

/**
 * Add the default palettes.
 *
 */
void
PaletteFile::addDefaultPalettes()
{
    bool modifiedStatus = this->isModified();
    
    this->addColor("none",  0xff, 0xff, 0xff );
    this->addColor("_yellow",  0xff, 0xff, 0x00 );
    this->addColor("_black",  0x00, 0x00, 0x00 );
    this->addColor("_orange",  0xff, 0x69, 0x00 );
    
    if (this->getPaletteByName(Palette::ROY_BIG_BL_PALETTE_NAME) == NULL) {
        this->addColor("_RGB_255_255_0",  255, 255, 0 ); //#ffff00
        this->addColor("_RGB_255_200_0",  255, 200, 0 ); //#ffc800
        this->addColor("_RGB_255_120_0",  255, 120, 0 ); //#ff7800
        this->addColor("_RGB_255_0_0",  255, 0, 0 ); //#ff0000
        this->addColor("_RGB_200_0_0",  200, 0, 0 ); //#c80000
        this->addColor("_RGB_150_0_0",  150, 0, 0 ); //#960000
        this->addColor("_RGB_100_0_0",  100, 0, 0 ); //#640000
        this->addColor("_RGB_60_0_0",  60, 0, 0 ); //#3c0000
        this->addColor("_RGB_0_0_80",  0, 0, 80 ); //#000050
        this->addColor("_RGB_0_0_170",  0, 0, 170 ); //#0000aa
        this->addColor("_RGB_75_0_125",  75, 0, 125 ); //#4b007d
        this->addColor("_RGB_125_0_160",  125, 0, 160 ); //#7d00a0
        this->addColor("_RGB_75_125_0",  75, 125, 0 ); //#4b7d00
        this->addColor("_RGB_0_200_0",  0, 200, 0 ); //#00c800
        this->addColor("_RGB_0_255_0",  0, 255, 0 ); //#00ff00
        this->addColor("_RGB_0_255_255",  0, 255, 255 ); //#00ffff
        
        Palette royBigBL;
        royBigBL.setName(Palette::ROY_BIG_BL_PALETTE_NAME);
        
        royBigBL.addScalarAndColor(1.00f, "_RGB_255_255_0");
        royBigBL.addScalarAndColor(0.875f, "_RGB_255_200_0");
        royBigBL.addScalarAndColor(0.750f, "_RGB_255_120_0");
        royBigBL.addScalarAndColor(0.625f, "_RGB_255_0_0");
        royBigBL.addScalarAndColor(0.500f, "_RGB_200_0_0");
        royBigBL.addScalarAndColor(0.375f, "_RGB_150_0_0");
        royBigBL.addScalarAndColor(0.250f, "_RGB_100_0_0");
        royBigBL.addScalarAndColor(0.125f, "_RGB_60_0_0");
        royBigBL.addScalarAndColor(0.000f, "_black");
        royBigBL.addScalarAndColor(-0.125f, "_RGB_0_0_80");
        royBigBL.addScalarAndColor(-0.250f, "_RGB_0_0_170");
        royBigBL.addScalarAndColor(-0.375f, "_RGB_75_0_125");
        royBigBL.addScalarAndColor(-0.500f, "_RGB_125_0_160");
        royBigBL.addScalarAndColor(-0.625f, "_RGB_75_125_0");
        royBigBL.addScalarAndColor(-0.750f, "_RGB_0_200_0");
        royBigBL.addScalarAndColor(-0.875f, "_RGB_0_255_0");
        royBigBL.addScalarAndColor(-0.990f, "_RGB_0_255_255");
        royBigBL.addScalarAndColor(-1.00f, "_RGB_0_255_255");
        
        addPalette(royBigBL);
    }
    
    //------------------------------------------------------------------------
    //
    // Palette by David Van Essen
    //
    int oran_yell[3] = { 0xff, 0x99, 0x00 };
    this->addColor("_oran-yell", oran_yell);
    int red[3] = { 0xff, 0x00, 0x00 };
    this->addColor("_red", red);
    int cyan[3] = { 0x00, 0xff, 0xff };
    this->addColor("_cyan", cyan);
    int green[3] = { 0x00, 0xff, 0x00 };
    this->addColor("_green", green);
    int limegreen[3] = { 0x10, 0xb0, 0x10 };
    this->addColor("_limegreen", limegreen);
    int violet[3] = { 0xe2, 0x51, 0xe2 };
    this->addColor("_violet", violet);
    int hotpink[3] = { 0xff, 0x38, 0x8d };
    this->addColor("_hotpink", hotpink);
    int white[3] = { 0xff, 0xff, 0xff };
    this->addColor("_white", white);
    int gry_dd[3] = { 0xdd, 0xdd, 0xdd };
    this->addColor("_gry-dd", gry_dd );
    int gry_bb[3] = { 0xbb, 0xbb, 0xbb };
    this->addColor("_gry-bb", gry_bb);
    int purple2[3] = { 0x66, 0x00, 0x33 };
    this->addColor("_purple2", purple2);
    int blue_videen11[3] = { 0x33, 0x33, 0x4c };
    this->addColor("_blue_videen11", blue_videen11);
    int blue_videen9[3] = { 0x4c, 0x4c, 0x7f };
    this->addColor("_blue_videen9", blue_videen9);
    int blue_videen7[3] = { 0x7f, 0x7f, 0xcc };
    this->addColor("_blue_videen7", blue_videen7);
    
    if (this->getPaletteByName("videen_style") == NULL) {
        Palette videenStyle;
        videenStyle.setName("videen_style");
        videenStyle.addScalarAndColor(1.0f, "_red");
        videenStyle.addScalarAndColor(0.9f, "_orange");
        videenStyle.addScalarAndColor(0.8f, "_oran-yell");
        videenStyle.addScalarAndColor(0.7f, "_yellow");
        videenStyle.addScalarAndColor(0.6f, "_limegreen");
        videenStyle.addScalarAndColor(0.5f, "_green");
        videenStyle.addScalarAndColor(0.4f, "_blue_videen7");
        videenStyle.addScalarAndColor(0.3f, "_blue_videen9");
        videenStyle.addScalarAndColor(0.2f, "_blue_videen11");
        videenStyle.addScalarAndColor(0.1f, "_purple2");
        videenStyle.addScalarAndColor(0.0f, "_black");
        videenStyle.addScalarAndColor(-0.1f, "_cyan");
        videenStyle.addScalarAndColor(-0.2f, "_green");
        videenStyle.addScalarAndColor(-0.3f, "_limegreen");
        videenStyle.addScalarAndColor(-0.4f, "_violet");
        videenStyle.addScalarAndColor(-0.5f, "_hotpink");
        videenStyle.addScalarAndColor(-0.6f, "_white");
        videenStyle.addScalarAndColor(-0.7f, "_gry-dd");
        videenStyle.addScalarAndColor(-0.8f, "_gry-bb");
        videenStyle.addScalarAndColor(-0.9f, "_black");
        addPalette(videenStyle);
    }
    
    //
    // Create a palette with just white and black designed to be used
    // with the interpolate option
    //   
    if (this->getPaletteByName(Palette::GRAY_INTERP_PALETTE_NAME) == NULL) {
        this->addColor("_white_gray_interp",  255, 255, 255 );
        this->addColor("_black_gray_interp",  0, 0, 0 );

        
        Palette palGrayPositiveInterp;
        palGrayPositiveInterp.setName(Palette::GRAY_INTERP_POSITIVE_PALETTE_NAME);        
        palGrayPositiveInterp.addScalarAndColor( 1.0f, "_white_gray_interp");
        palGrayPositiveInterp.addScalarAndColor(0.0f, "_black_gray_interp");
        addPalette(palGrayPositiveInterp);   
        
        Palette palGrayInterp;
        palGrayInterp.setName(Palette::GRAY_INTERP_PALETTE_NAME);        
        palGrayInterp.addScalarAndColor( 1.0f, "_white_gray_interp");
        palGrayInterp.addScalarAndColor(-1.0f, "_black_gray_interp");
        addPalette(palGrayInterp);   
    }
    
    //----------------------------------------------------------------------
    // fixed Psych palette
    //
    if (this->getPaletteByName("PSYCH-FIXED") == NULL) {
        this->addColor("_pyell-oran",  0xff, 0xcc, 0x00 );
        this->addColor("_poran-red",  0xff, 0x44, 0x00 );
        this->addColor("_pred",  0xff, 0x00, 0x00 );
        this->addColor("_pblue",  0x00, 0x44, 0xff );
        this->addColor("_pltblue1",  0x00, 0x69, 0xff );
        this->addColor("_pltblue2",  0x00, 0x99, 0xff );
        this->addColor("_pbluecyan",  0x00, 0xcc, 0xff );
        this->addColor("_pcyan",  0x00, 0xff, 0xff );
        
        Palette psychFixed;
        psychFixed.setName("PSYCH-FIXED");
        //psych.setPositiveOnly(false);
        
        psychFixed.addScalarAndColor(1.00f, "_yellow");
        psychFixed.addScalarAndColor(0.75f, "_pyell-oran");
        psychFixed.addScalarAndColor(0.50f, "_orange");
        psychFixed.addScalarAndColor(0.25f, "_poran-red");
        psychFixed.addScalarAndColor(0.00001f, "_pred");//0.00001f is a special range reserved by data normalization for zero values, see PaletteColorMapping.cxx:1590
        psychFixed.addScalarAndColor(0.0000099f, "_black");
        psychFixed.addScalarAndColor(-0.0000099f, "_black");
        psychFixed.addScalarAndColor(-0.00001f, "_pblue");
        psychFixed.addScalarAndColor(-0.25f, "_pltblue1");
        psychFixed.addScalarAndColor(-0.50f, "_pltblue2");
        psychFixed.addScalarAndColor(-0.75f, "_pbluecyan");
        psychFixed.addScalarAndColor(-1.0f, "_pcyan");
        
        addPalette(psychFixed);
    }
    
    //------------------------------------------------------------------------
    //
    // Palette by Jon Wieser @ mcw
    //
    int rbgyr20_01[3] = { 0xCC, 0x10, 0x33 };
    this->addColor("_rbgyr20_01", rbgyr20_01);
    int rbgyr20_02[3] = { 0x99, 0x20, 0x66 };
    this->addColor("_rbgyr20_02", rbgyr20_02);
    int rbgyr20_03[3] = { 0x66, 0x31, 0x99 };
    this->addColor("_rbgyr20_03", rbgyr20_03);
    int rbgyr20_04[3] = { 0x34, 0x41, 0xCC };
    this->addColor("_rbgyr20_04", rbgyr20_04);
    int rbgyr20_05[3] = { 0x00, 0x51, 0xFF };
    this->addColor("_rbgyr20_05", rbgyr20_05);
    int rbgyr20_06[3] = { 0x00, 0x74, 0xCC };
    this->addColor("_rbgyr20_06", rbgyr20_06);
    int rbgyr20_07[3] = { 0x00, 0x97, 0x99 };
    this->addColor("_rbgyr20_07", rbgyr20_07);
    int rbgyr20_08[3] = { 0x00, 0xB9, 0x66 };
    this->addColor("_rbgyr20_08", rbgyr20_08);
    int rbgyr20_09[3] = { 0x00, 0xDC, 0x33 };
    this->addColor("_rbgyr20_09", rbgyr20_09);
    int rbgyr20_10[3] = { 0x00, 0xFF, 0x00 };
    this->addColor("_rbgyr20_10", rbgyr20_10);
    int rbgyr20_11[3] = { 0x33, 0xFF, 0x00 };
    this->addColor("_rbgyr20_11", rbgyr20_11);
    int rbgyr20_12[3] = { 0x66, 0xFF, 0x00 };
    this->addColor("_rbgyr20_12", rbgyr20_12);
    int rbgyr20_13[3] = { 0x99, 0xFF, 0x00 };
    this->addColor("_rbgyr20_13", rbgyr20_13);
    int rbgyr20_14[3] = { 0xCC, 0xFF, 0x00 };
    this->addColor("_rbgyr20_14", rbgyr20_14);
    int rbgyr20_15[3] = { 0xFF, 0xFF, 0x00 };
    this->addColor("_rbgyr20_15", rbgyr20_15);
    int rbgyr20_16[3] = { 0xFF, 0xCC, 0x00 };
    this->addColor("_rbgyr20_16", rbgyr20_16);
    int rbgyr20_17[3] = { 0xFF, 0x99, 0x00 };
    this->addColor("_rbgyr20_17", rbgyr20_17);
    int rbgyr20_18[3] = { 0xFF, 0x66, 0x00 };
    this->addColor("_rbgyr20_18", rbgyr20_18);
    int rbgyr20_19[3] = { 0xFF, 0x33, 0x00 };
    this->addColor("_rbgyr20_19", rbgyr20_19);
    int rbgyr20_20[3] = { 0xFF, 0x00, 0x00 };
    this->addColor("_rbgyr20_20", rbgyr20_20);
    
    if (this->getPaletteByName("RBGYR20") == NULL) {
        Palette pal2;
        pal2.setName("RBGYR20");
        pal2.addScalarAndColor( 1.0f, "_rbgyr20_01");
        pal2.addScalarAndColor( 0.9f, "_rbgyr20_02");
        pal2.addScalarAndColor( 0.8f, "_rbgyr20_03");
        pal2.addScalarAndColor( 0.7f, "_rbgyr20_04");
        pal2.addScalarAndColor( 0.6f, "_rbgyr20_05");
        pal2.addScalarAndColor( 0.5f, "_rbgyr20_06");
        pal2.addScalarAndColor( 0.4f, "_rbgyr20_07");
        pal2.addScalarAndColor( 0.3f, "_rbgyr20_08");
        pal2.addScalarAndColor( 0.2f, "_rbgyr20_09");
        pal2.addScalarAndColor( 0.1f, "_rbgyr20_10");
        pal2.addScalarAndColor( 0.0f, "_rbgyr20_11");
        pal2.addScalarAndColor(-0.1f, "_rbgyr20_12");
        pal2.addScalarAndColor(-0.2f, "_rbgyr20_13");
        pal2.addScalarAndColor(-0.3f, "_rbgyr20_14");
        pal2.addScalarAndColor(-0.4f, "_rbgyr20_15");
        pal2.addScalarAndColor(-0.5f, "_rbgyr20_16");
        pal2.addScalarAndColor(-0.6f, "_rbgyr20_17");
        pal2.addScalarAndColor(-0.7f, "_rbgyr20_18");
        pal2.addScalarAndColor(-0.8f, "_rbgyr20_19");
        pal2.addScalarAndColor(-0.9f, "_rbgyr20_20");
        addPalette(pal2);
        
        Palette pal3;
        pal3.setName("RBGYR20P");
        pal3.addScalarAndColor(1.00f, "_rbgyr20_01");
        pal3.addScalarAndColor(0.95f, "_rbgyr20_02");
        pal3.addScalarAndColor(0.90f, "_rbgyr20_03");
        pal3.addScalarAndColor(0.85f, "_rbgyr20_04");
        pal3.addScalarAndColor(0.80f, "_rbgyr20_05");
        pal3.addScalarAndColor(0.75f, "_rbgyr20_06");
        pal3.addScalarAndColor(0.70f, "_rbgyr20_07");
        pal3.addScalarAndColor(0.65f, "_rbgyr20_08");
        pal3.addScalarAndColor(0.60f, "_rbgyr20_09");
        pal3.addScalarAndColor(0.55f, "_rbgyr20_10");
        pal3.addScalarAndColor(0.50f, "_rbgyr20_11");
        pal3.addScalarAndColor(0.45f, "_rbgyr20_12");
        pal3.addScalarAndColor(0.40f, "_rbgyr20_13");
        pal3.addScalarAndColor(0.35f, "_rbgyr20_14");
        pal3.addScalarAndColor(0.30f, "_rbgyr20_15");
        pal3.addScalarAndColor(0.25f, "_rbgyr20_16");
        pal3.addScalarAndColor(0.20f, "_rbgyr20_17");
        pal3.addScalarAndColor(0.15f, "_rbgyr20_18");
        pal3.addScalarAndColor(0.10f, "_rbgyr20_19");
        pal3.addScalarAndColor(0.05f, "_rbgyr20_20");
        pal3.addScalarAndColor(0.0f, "none");
        addPalette(pal3);
    }
    
    if (this->getPaletteByName("RYGBR4_positive") == NULL) {
        this->addColor("rygbr4_0", 255, 0, 0);
        this->addColor("rygbr4_1", 255, 255, 0);
        this->addColor("rygbr4_2", 0, 180, 0);
        this->addColor("rygbr4_3", 0, 0, 255);
        
        Palette rygbr4_pos;
        rygbr4_pos.setName("RYGBR4_positive");
        rygbr4_pos.addScalarAndColor(1.0f, "rygbr4_0");
        rygbr4_pos.addScalarAndColor(0.75f, "rygbr4_3");
        rygbr4_pos.addScalarAndColor(0.5f, "rygbr4_2");
        rygbr4_pos.addScalarAndColor(0.25f, "rygbr4_1");
        rygbr4_pos.addScalarAndColor(0.0f, "rygbr4_0");
        rygbr4_pos.addScalarAndColor(-1.0f, "rygbr4_0");//negatives red, I guess
        addPalette(rygbr4_pos);
    }
    
    if (this->getPaletteByName("RGRBR_mirror90_pos") == NULL) {
        this->addColor("rgrbr_m9_p_red", 220, 0, 0);
        this->addColor("rgrbr_m9_p_yellow", 255, 255, 0);
        this->addColor("rgrbr_m9_p_green", 0, 180, 0);
        this->addColor("rgrbr_m9_p_blue", 0, 0, 255);
        this->addColor("rgrbr_m9_p_purple", 255, 100, 255);
        
        Palette rgbgr_m9_p;
        rgbgr_m9_p.setName("RGRBR_mirror90_pos");
        rgbgr_m9_p.addScalarAndColor(1.0f, "rgrbr_m9_p_red");
        rgbgr_m9_p.addScalarAndColor(0.875f, "rgrbr_m9_p_purple");
        rgbgr_m9_p.addScalarAndColor(0.75f, "rgrbr_m9_p_blue");
        rgbgr_m9_p.addScalarAndColor(0.625f, "rgrbr_m9_p_purple");
        rgbgr_m9_p.addScalarAndColor(0.5f, "rgrbr_m9_p_red");
        rgbgr_m9_p.addScalarAndColor(0.375f, "rgrbr_m9_p_yellow");
        rgbgr_m9_p.addScalarAndColor(0.25f, "rgrbr_m9_p_green");
        rgbgr_m9_p.addScalarAndColor(0.125f, "rgrbr_m9_p_yellow");
        rgbgr_m9_p.addScalarAndColor(0.0f, "rgrbr_m9_p_red");
        rgbgr_m9_p.addScalarAndColor(-1.0f, "rgrbr_m9_p_red");//negatives red, I guess
        addPalette(rgbgr_m9_p);

    }
    
    //----------------------------------------------------------------------
    // Orange-Yellow palette
    //
    if (this->getPaletteByName("Orange-Yellow") == NULL) {
        this->addColor("_oy1",  0, 0, 0 );
        this->addColor("_oy2",  130, 2, 0 );
        this->addColor("_oy3",  254, 130, 2 );
        this->addColor("_oy4",  254, 254, 126 );
        this->addColor("_oy5",  254, 254, 254 );
        
        Palette orangeYellow;
        orangeYellow.setName("Orange-Yellow");
        orangeYellow.addScalarAndColor( 1.0f, "_oy5");
        orangeYellow.addScalarAndColor( 0.5f, "_oy4");
        orangeYellow.addScalarAndColor( 0.0f, "_oy3");
        orangeYellow.addScalarAndColor(-0.5f, "_oy2");
        orangeYellow.addScalarAndColor(-1.0f, "_oy1");
        addPalette(orangeYellow);
    }
    
    //----------------------------------------------------------------------
    // Positive/Negative/Zero palette
    //
    if (this->getPaletteByName("POS_NEG_ZERO") == NULL) {
        this->addColor("pos_neg_blue",  0x00, 0x00, 0xff );
        this->addColor("pos_neg_red",  0xff, 0x00, 0x00 );
        
        Palette posNegZero;
        posNegZero.setName("POS_NEG_ZERO");
        
        posNegZero.addScalarAndColor(1.0f, "pos_neg_red");
        posNegZero.addScalarAndColor(0.00001f, "pos_neg_red");
        posNegZero.addScalarAndColor(0.0000099f, "_black");
        posNegZero.addScalarAndColor(-0.0000099f, "_black");
        posNegZero.addScalarAndColor(-0.00001f, "pos_neg_blue");
        posNegZero.addScalarAndColor(-1.0f, "pos_neg_blue");
        
        addPalette(posNegZero);
    }
    
    if (this->getPaletteByName("red-yellow") == NULL) {
        this->addColor("_red_yellow_interp_red",  255, 0, 0 );
        this->addColor("_red_yellow_interp_yellow",  255, 255, 0 );
        this->addColor("_blue_lightblue_interp_blue",  0, 0, 255 );
        this->addColor("_blue_lightblue_interp_lightblue",  0, 255, 255 );
        this->addColor("_fslview_zero", 0, 0, 0);

        Palette palRedYellowInterp;
        palRedYellowInterp.setName("red-yellow");
        palRedYellowInterp.addScalarAndColor(1.0f, "_red_yellow_interp_yellow");
        palRedYellowInterp.addScalarAndColor(0.0f, "_red_yellow_interp_red");
        addPalette(palRedYellowInterp);

        Palette palBlueLightblueInterp;
        palBlueLightblueInterp.setName("blue-lightblue");
        palBlueLightblueInterp.addScalarAndColor(1.0f, "_blue_lightblue_interp_lightblue");
        palBlueLightblueInterp.addScalarAndColor(0.0f, "_blue_lightblue_interp_blue");
        addPalette(palBlueLightblueInterp);
        
        Palette palFSLView;
        palFSLView.setName("FSL");
        palFSLView.addScalarAndColor( 1.0f, "_red_yellow_interp_yellow");
        palFSLView.addScalarAndColor( 0.00001f, "_red_yellow_interp_red");
        palFSLView.addScalarAndColor( 0.0000099f, "_fslview_zero");
        palFSLView.addScalarAndColor(-0.0000099f, "_fslview_zero");
        palFSLView.addScalarAndColor(-0.00001f, "_blue_lightblue_interp_blue");
        palFSLView.addScalarAndColor(-1.0f, "_blue_lightblue_interp_lightblue");
        addPalette(palFSLView);
    }

    if (this->getPaletteByName("power_surf") == NULL) {
        this->addColor("_ps_0",    1.0 *255.0,   0.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_059",  0.0 * 255.0,  0.0 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_118",  1.0 * 255.0,  1.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_176",  1.0 * 255.0,  0.7 * 255.0,  0.4 * 255.0);
        this->addColor("_ps_235",  0.0 * 255.0,  0.8 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_294",  1.0 * 255.0,  0.6 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_353",  0.0 * 255.0,  0.6 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_412",  0.0 * 255.0,  0.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_471",  0.3 * 255.0,  0.0 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_529",  0.2 * 255.0,  1.0 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_588",  1.0 * 255.0,  0.5 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_647",  0.6 * 255.0,  0.2 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_706",  0.0 * 255.0,  0.2 * 255.0,  0.4 * 255.0 );
        this->addColor("_ps_765",  0.2 * 255.0,  1.0 * 255.0,  0.2 * 255.0 );
        this->addColor("_ps_824",  0.0 * 255.0,  0.0 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_882",  1.0 * 255.0,  1.0 * 255.0,  0.8 * 255.0 );
        this->addColor("_ps_941",  0.0 * 255.0,  0.4 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_1000", 0.25 * 255.0, 0.25 * 255.0, 0.25 * 255.0 );
        
        Palette powerSurf;
        powerSurf.setName("power_surf");
        powerSurf.addScalarAndColor( 1.0, "_ps_1000");
        powerSurf.addScalarAndColor( 0.941, "_ps_941");
        powerSurf.addScalarAndColor( 0.882, "_ps_882");
        powerSurf.addScalarAndColor( 0.824, "_ps_824");
        powerSurf.addScalarAndColor( 0.765, "_ps_765");
        powerSurf.addScalarAndColor( 0.706, "_ps_706");
        powerSurf.addScalarAndColor( 0.647, "_ps_647");
        powerSurf.addScalarAndColor( 0.588, "_ps_588");
        powerSurf.addScalarAndColor( 0.529, "_ps_529");
        powerSurf.addScalarAndColor( 0.471, "_ps_471");
        powerSurf.addScalarAndColor( 0.412, "_ps_412");
        powerSurf.addScalarAndColor( 0.353, "_ps_353");
        powerSurf.addScalarAndColor( 0.294, "_ps_294");
        powerSurf.addScalarAndColor( 0.235, "_ps_235");
        powerSurf.addScalarAndColor( 0.176, "_ps_176");
        powerSurf.addScalarAndColor( 0.118, "_ps_118");
        powerSurf.addScalarAndColor( 0.059, "_ps_059");
        powerSurf.addScalarAndColor( 0.0, "_ps_0");
        addPalette(powerSurf);
    }

    //black to color versions of three of the fsl palettes below
    if (this->getPaletteByName("black-red") == NULL) {
        Palette blackRed;
        blackRed.setName("black-red");
        this->addColor("black-red_0", 0, 0, 0);
        this->addColor("black-red_05", 127, 0, 0); //provide a zero color for neg to pos palette, for sanity
        this->addColor("black-red_1", 255, 0, 0);
        blackRed.addScalarAndColor(1.0f, "black-red_1");
        blackRed.addScalarAndColor(0.0f, "black-red_05");
        blackRed.addScalarAndColor(-1.0f, "black-red_0");
        addPalette(blackRed);

        Palette blackGreen;
        blackGreen.setName("black-green");
        this->addColor("black-green_0", 0, 0, 0);
        this->addColor("black-green_05", 0, 127, 0);
        this->addColor("black-green_1", 0, 255, 0);
        //this->addColor("black-green_05", 0, 90, 0); //experimental option to try to match brightness across color options
        //this->addColor("black-green_1", 0, 180, 0);
        blackGreen.addScalarAndColor(1.0f, "black-green_1");
        blackGreen.addScalarAndColor(0.0f, "black-green_05");
        blackGreen.addScalarAndColor(-1.0f, "black-green_0");
        addPalette(blackGreen);

        Palette blackBlue;
        blackBlue.setName("black-blue");
        this->addColor("black-blue_0", 0, 0, 0);
        this->addColor("black-blue_05", 0, 0, 127);
        this->addColor("black-blue_1", 0, 0, 255);
        //this->addColor("black-blue_05", 50, 50, 127); //experimental option to try to match brightness across color options
        //this->addColor("black-blue_1", 100, 100, 255);
        blackBlue.addScalarAndColor(1.0f, "black-blue_1");
        blackBlue.addScalarAndColor(0.0f, "black-blue_05");
        blackBlue.addScalarAndColor(-1.0f, "black-blue_0");
        addPalette(blackBlue);

        Palette blackRedPos;
        blackRedPos.setName("black-red-positive");
        blackRedPos.addScalarAndColor(1.0f, "black-red_1");
        blackRedPos.addScalarAndColor(0.0f, "black-red_0");
        addPalette(blackRedPos);
        
        Palette blackGreenPositive;
        blackGreenPositive.setName("black-green-positive");
        blackGreenPositive.addScalarAndColor(1.0f, "black-green_1");
        blackGreenPositive.addScalarAndColor(0.0f, "black-green_0");
        addPalette(blackGreenPositive);
        
        Palette blackBluePositive;
        blackBluePositive.setName("black-blue-positive");
        blackBluePositive.addScalarAndColor(1.0f, "black-blue_1");
        blackBluePositive.addScalarAndColor(0.0f, "black-blue_0");
        addPalette(blackBluePositive);
    }

    if (this->getPaletteByName("blue-black-green") == NULL) {
        Palette bbg;
        bbg.setName("blue-black-green");
        
        addPaletteScalarAndColor(bbg,  1.0, 0, 255,   0); /* green */
        addPaletteScalarAndColor(bbg,  0.0, 0,   0,   0); /* black */
        addPaletteScalarAndColor(bbg, -1.0, 0,   0, 255); /* blue */

        addPalette(bbg);
    }
    
    if (this->getPaletteByName("blue-black-red") == NULL) {
        Palette bbr;
        bbr.setName("blue-black-red");
        
        addPaletteScalarAndColor(bbr,  1.0, 255, 0,   0); /* red */
        addPaletteScalarAndColor(bbr,  0.0,   0, 0,   0); /* black */
        addPaletteScalarAndColor(bbr, -1.0,   0, 0, 255); /* blue */
        
        addPalette(bbr);
    }
    
    if (this->getPaletteByName("red-black-green") == NULL) {
        Palette rbg;
        rbg.setName("red-black-green");
        
        addPaletteScalarAndColor(rbg,  1.0,   0, 255, 0); /* green */
        addPaletteScalarAndColor(rbg,  0.0,   0,   0, 0); /* black */
        addPaletteScalarAndColor(rbg, -1.0, 255,   0, 0); /* red */
        
        addPalette(rbg);
    }
    
    /*
     * FSL Red palette from WB-289
     *
     * float offset = 100.0;
     * float step = (255.0 - offset) / 255.0;
     * for(unsigned char i = 0; i < 255; ++i)
     * { int red = int(((i + 1) * step) + offset); lut->pushValue(red, 0, 0, i); }
     *
     * lut->m_lutName = std::string("Red");
     */
    //TSC: no "lookup tables" for purely interpolated palettes!  bad for performance.
    if (this->getPaletteByName("fsl_red") == NULL) {
        Palette fslRed;
        fslRed.setName("fsl_red");
        this->addColor("fsl_red_0", 100, 0, 0);
        this->addColor("fsl_red_1", 255, 0, 0);
        fslRed.addScalarAndColor(1.0f, "fsl_red_1");
        fslRed.addScalarAndColor(0.0f, "fsl_red_0");
        addPalette(fslRed);
    }
    
    if (this->getPaletteByName("fsl_green") == NULL) {
        Palette fslGreen;
        fslGreen.setName("fsl_green");
        this->addColor("fsl_green_0", 0, 100, 0);
        this->addColor("fsl_green_1", 0, 255, 0);
        fslGreen.addScalarAndColor(1.0f, "fsl_green_1");
        fslGreen.addScalarAndColor(0.0f, "fsl_green_0");
        addPalette(fslGreen);
    }
    
    if (this->getPaletteByName("fsl_blue") == NULL) {
        Palette fslBlue;
        fslBlue.setName("fsl_blue");
        this->addColor("fsl_blue_0", 0, 0, 100);
        this->addColor("fsl_blue_1", 0, 0, 255);
        fslBlue.addScalarAndColor(1.0f, "fsl_blue_1");
        fslBlue.addScalarAndColor(0.0f, "fsl_blue_0");
        addPalette(fslBlue);
    }
    
    if (this->getPaletteByName("fsl_yellow") == NULL) {
        Palette fslYellow;
        fslYellow.setName("fsl_yellow");
        this->addColor("fsl_yellow_0", 100, 100, 0);
        this->addColor("fsl_yellow_1", 255, 255, 0);
        fslYellow.addScalarAndColor(1.0f, "fsl_yellow_1");
        fslYellow.addScalarAndColor(0.0f, "fsl_yellow_0");
        addPalette(fslYellow);
    }
    
    //
    // Create a palette with red (positive), white (zero),
    // and blue (negative) for Alan A.
    //
    if (this->getPaletteByName("RedWhiteBlue") == NULL) {
        Palette redWhiteBlue;
        redWhiteBlue.setName("RedWhiteBlue");
        this->addColor("rwbBlue", 0, 0, 255);
        this->addColor("rwbBlueMiddle", 127, 127, 255);
        this->addColor("rwbWhite", 255, 255, 255);
        this->addColor("rwbRedMiddle", 255, 127, 127);
        this->addColor("rwbRed", 255, 0, 0);
        redWhiteBlue.addScalarAndColor( 1.0f, "rwbRed");
        redWhiteBlue.addScalarAndColor( 0.5f, "rwbRedMiddle");
        redWhiteBlue.addScalarAndColor( 0.0f, "rwbWhite");
        redWhiteBlue.addScalarAndColor(-0.5f, "rwbBlueMiddle");
        redWhiteBlue.addScalarAndColor(-1.0f, "rwbBlue");
        addPalette(redWhiteBlue);
    }
    
    //coolwarm, http://www.kennethmoreland.com/color-maps/
    //"Diverging Color Maps for Scientific Visualization." Kenneth Moreland. In Proceedings of the 5th International Symposium on Visual Computing, December 2009. DOI 10.1007/978-3-642-10520-3_9.
    //this palette was interpolated in CIELAB space, and then translated to sRGB, so we need to keep a substantial number of points
    //use the 33 csv file
    if (this->getPaletteByName("cool-warm") == NULL) {
        Palette coolwarm;
        coolwarm.setName("cool-warm");
        this->addColor("cool-warm-0", 59,76,192);
        this->addColor("cool-warm-1", 68,90,204);
        this->addColor("cool-warm-2", 77,104,215);
        this->addColor("cool-warm-3", 87,117,225);
        this->addColor("cool-warm-4", 98,130,234);
        this->addColor("cool-warm-5", 108,142,241);
        this->addColor("cool-warm-6", 119,154,247);
        this->addColor("cool-warm-7", 130,165,251);
        this->addColor("cool-warm-8", 141,176,254);
        this->addColor("cool-warm-9", 152,185,255);
        this->addColor("cool-warm-10", 163,194,255);
        this->addColor("cool-warm-11", 174,201,253);
        this->addColor("cool-warm-12", 184,208,249);
        this->addColor("cool-warm-13", 194,213,244);
        this->addColor("cool-warm-14", 204,217,238);
        this->addColor("cool-warm-15", 213,219,230);
        this->addColor("cool-warm-16", 221,221,221);
        this->addColor("cool-warm-17", 229,216,209);
        this->addColor("cool-warm-18", 236,211,197);
        this->addColor("cool-warm-19", 241,204,185);
        this->addColor("cool-warm-20", 245,196,173);
        this->addColor("cool-warm-21", 247,187,160);
        this->addColor("cool-warm-22", 247,177,148);
        this->addColor("cool-warm-23", 247,166,135);
        this->addColor("cool-warm-24", 244,154,123);
        this->addColor("cool-warm-25", 241,141,111);
        this->addColor("cool-warm-26", 236,127,99);
        this->addColor("cool-warm-27", 229,112,88);
        this->addColor("cool-warm-28", 222,96,77);
        this->addColor("cool-warm-29", 213,80,66);
        this->addColor("cool-warm-30", 203,62,56);
        this->addColor("cool-warm-31", 192,40,47);
        this->addColor("cool-warm-32", 180,4,38);
        coolwarm.addScalarAndColor(16.0f / 16.0f, "cool-warm-32");
        coolwarm.addScalarAndColor(15.0f / 16.0f, "cool-warm-31");
        coolwarm.addScalarAndColor(14.0f / 16.0f, "cool-warm-30");
        coolwarm.addScalarAndColor(13.0f / 16.0f, "cool-warm-29");
        coolwarm.addScalarAndColor(12.0f / 16.0f, "cool-warm-28");
        coolwarm.addScalarAndColor(11.0f / 16.0f, "cool-warm-27");
        coolwarm.addScalarAndColor(10.0f / 16.0f, "cool-warm-26");
        coolwarm.addScalarAndColor(9.0f / 16.0f, "cool-warm-25");
        coolwarm.addScalarAndColor(8.0f / 16.0f, "cool-warm-24");
        coolwarm.addScalarAndColor(7.0f / 16.0f, "cool-warm-23");
        coolwarm.addScalarAndColor(6.0f / 16.0f, "cool-warm-22");
        coolwarm.addScalarAndColor(5.0f / 16.0f, "cool-warm-21");
        coolwarm.addScalarAndColor(4.0f / 16.0f, "cool-warm-20");
        coolwarm.addScalarAndColor(3.0f / 16.0f, "cool-warm-19");
        coolwarm.addScalarAndColor(2.0f / 16.0f, "cool-warm-18");
        coolwarm.addScalarAndColor(1.0f / 16.0f, "cool-warm-17");
        coolwarm.addScalarAndColor(0.0f / 16.0f, "cool-warm-16");
        coolwarm.addScalarAndColor(-1.0f / 16.0f, "cool-warm-15");
        coolwarm.addScalarAndColor(-2.0f / 16.0f, "cool-warm-14");
        coolwarm.addScalarAndColor(-3.0f / 16.0f, "cool-warm-13");
        coolwarm.addScalarAndColor(-4.0f / 16.0f, "cool-warm-12");
        coolwarm.addScalarAndColor(-5.0f / 16.0f, "cool-warm-11");
        coolwarm.addScalarAndColor(-6.0f / 16.0f, "cool-warm-10");
        coolwarm.addScalarAndColor(-7.0f / 16.0f, "cool-warm-9");
        coolwarm.addScalarAndColor(-8.0f / 16.0f, "cool-warm-8");
        coolwarm.addScalarAndColor(-9.0f / 16.0f, "cool-warm-7");
        coolwarm.addScalarAndColor(-10.0f / 16.0f, "cool-warm-6");
        coolwarm.addScalarAndColor(-11.0f / 16.0f, "cool-warm-5");
        coolwarm.addScalarAndColor(-12.0f / 16.0f, "cool-warm-4");
        coolwarm.addScalarAndColor(-13.0f / 16.0f, "cool-warm-3");
        coolwarm.addScalarAndColor(-14.0f / 16.0f, "cool-warm-2");
        coolwarm.addScalarAndColor(-15.0f / 16.0f, "cool-warm-1");
        coolwarm.addScalarAndColor(-16.0f / 16.0f, "cool-warm-0");
        addPalette(coolwarm);
    }
    
    //matplotlib's spectral is from colorbrewer
    //Colors from www.ColorBrewer.org by Cynthia A. Brewer, Geography, Pennsylvania State University.
    //Copyright (c) 2002 Cynthia Brewer, Mark Harrower, and The Pennsylvania State University.
    if (this->getPaletteByName("spectral") == NULL) {
        Palette spectral;
        spectral.setName("spectral");
        this->addColor("spectral-0", 158,1,66);
        this->addColor("spectral-1", 213,62,79);
        this->addColor("spectral-2", 244,109,67);
        this->addColor("spectral-3", 253,174,97);
        this->addColor("spectral-4", 254,224,139);
        this->addColor("spectral-5", 255,255,191);
        this->addColor("spectral-6", 230,245,152);
        this->addColor("spectral-7", 171,221,164);
        this->addColor("spectral-8", 102,194,165);
        this->addColor("spectral-9", 50,136,189);
        this->addColor("spectral-10", 94,79,162);
        spectral.addScalarAndColor(5.0f / 5.0f, "spectral-0");
        spectral.addScalarAndColor(4.0f / 5.0f, "spectral-1");
        spectral.addScalarAndColor(3.0f / 5.0f, "spectral-2");
        spectral.addScalarAndColor(2.0f / 5.0f, "spectral-3");
        spectral.addScalarAndColor(1.0f / 5.0f, "spectral-4");
        spectral.addScalarAndColor(0.0f / 5.0f, "spectral-5");
        spectral.addScalarAndColor(-1.0f / 5.0f, "spectral-6");
        spectral.addScalarAndColor(-2.0f / 5.0f, "spectral-7");
        spectral.addScalarAndColor(-3.0f / 5.0f, "spectral-8");
        spectral.addScalarAndColor(-4.0f / 5.0f, "spectral-9");
        spectral.addScalarAndColor(-5.0f / 5.0f, "spectral-10");
        addPalette(spectral);
    }
    
    //palette from Alan Anticevic
    if (this->getPaletteByName("RY-BC-BL") == NULL) {
        Palette RY_BC_BL;
        RY_BC_BL.setName("RY-BC-BL");//named like "ROY-BIG-BL"
        this->addColor("rybcbl-y", 255, 255, 0);
        this->addColor("rybcbl-r", 255, 0, 0);
        this->addColor("rybcbl-bl", 0, 0, 0);
        this->addColor("rybcbl-b", 0, 0, 255);
        this->addColor("rybcbl-c", 0, 255, 255);
        RY_BC_BL.addScalarAndColor(1.0f, "rybcbl-y");
        RY_BC_BL.addScalarAndColor(0.5f, "rybcbl-r");
        RY_BC_BL.addScalarAndColor(0.0f, "rybcbl-bl");
        RY_BC_BL.addScalarAndColor(-0.5f, "rybcbl-b");
        RY_BC_BL.addScalarAndColor(-1.0f, "rybcbl-c");
        addPalette(RY_BC_BL);
    }
    
    //matplotlib's magma, approximated by reducing their massive 256-entry lookup table to 32 entries
    //Copyright (c) 2012-2015 Matplotlib Development Team; All Rights Reserved
    //see https://github.com/matplotlib/matplotlib/blob/master/LICENSE/LICENSE or debian/copyright for the matplotlib license, "All Rights Reserved" is required language by the license, but it is actually open source
    if (this->getPaletteByName("magma") == NULL) {
        Palette magma;
        magma.setName("magma");
        this->addColor("magma-black", 0, 0, 0);//negative and zero
        this->addColor("magma-1", 3, 3, 15);
        this->addColor("magma-2", 9, 7, 32);
        this->addColor("magma-3", 18, 13, 49);
        this->addColor("magma-4", 28, 16, 68);
        this->addColor("magma-5", 39, 18, 88);
        this->addColor("magma-6", 52, 16, 105);
        this->addColor("magma-7", 66, 15, 117);
        this->addColor("magma-8", 79, 18, 123);
        this->addColor("magma-9", 92, 22, 127);
        this->addColor("magma-10", 104, 28, 129);
        this->addColor("magma-11", 117, 33, 129);
        this->addColor("magma-12", 129, 37, 129);
        this->addColor("magma-13", 142, 42, 129);
        this->addColor("magma-14", 155, 46, 127);
        this->addColor("magma-15", 168, 50, 125);
        this->addColor("magma-16", 181, 54, 122);
        this->addColor("magma-17", 194, 59, 117);
        this->addColor("magma-18", 207, 64, 112);
        this->addColor("magma-19", 219, 71, 106);
        this->addColor("magma-20", 229, 80, 100);
        this->addColor("magma-21", 238, 91, 94);
        this->addColor("magma-22", 244, 105, 92);
        this->addColor("magma-23", 249, 120, 93);
        this->addColor("magma-24", 251, 135, 97);
        this->addColor("magma-25", 253, 150, 104);
        this->addColor("magma-26", 254, 165, 113);
        this->addColor("magma-27", 254, 180, 123);
        this->addColor("magma-28", 254, 194, 135);
        this->addColor("magma-29", 254, 209, 148);
        this->addColor("magma-30", 253, 224, 161);
        this->addColor("magma-31", 252, 238, 176);
        this->addColor("magma-32", 252, 253, 191);
        magma.addScalarAndColor(32.0f / 32.0f, "magma-32");
        magma.addScalarAndColor(31.0f / 32.0f, "magma-31");
        magma.addScalarAndColor(30.0f / 32.0f, "magma-30");
        magma.addScalarAndColor(29.0f / 32.0f, "magma-29");
        magma.addScalarAndColor(28.0f / 32.0f, "magma-28");
        magma.addScalarAndColor(27.0f / 32.0f, "magma-27");
        magma.addScalarAndColor(26.0f / 32.0f, "magma-26");
        magma.addScalarAndColor(25.0f / 32.0f, "magma-25");
        magma.addScalarAndColor(24.0f / 32.0f, "magma-24");
        magma.addScalarAndColor(23.0f / 32.0f, "magma-23");
        magma.addScalarAndColor(22.0f / 32.0f, "magma-22");
        magma.addScalarAndColor(21.0f / 32.0f, "magma-21");
        magma.addScalarAndColor(20.0f / 32.0f, "magma-20");
        magma.addScalarAndColor(19.0f / 32.0f, "magma-19");
        magma.addScalarAndColor(18.0f / 32.0f, "magma-18");
        magma.addScalarAndColor(17.0f / 32.0f, "magma-17");
        magma.addScalarAndColor(16.0f / 32.0f, "magma-16");
        magma.addScalarAndColor(15.0f / 32.0f, "magma-15");
        magma.addScalarAndColor(14.0f / 32.0f, "magma-14");
        magma.addScalarAndColor(13.0f / 32.0f, "magma-13");
        magma.addScalarAndColor(12.0f / 32.0f, "magma-12");
        magma.addScalarAndColor(11.0f / 32.0f, "magma-11");
        magma.addScalarAndColor(10.0f / 32.0f, "magma-10");
        magma.addScalarAndColor(9.0f / 32.0f, "magma-9");
        magma.addScalarAndColor(8.0f / 32.0f, "magma-8");
        magma.addScalarAndColor(7.0f / 32.0f, "magma-7");
        magma.addScalarAndColor(6.0f / 32.0f, "magma-6");
        magma.addScalarAndColor(5.0f / 32.0f, "magma-5");
        magma.addScalarAndColor(4.0f / 32.0f, "magma-4");
        magma.addScalarAndColor(3.0f / 32.0f, "magma-3");
        magma.addScalarAndColor(2.0f / 32.0f, "magma-2");
        magma.addScalarAndColor(1.0f / 32.0f, "magma-1");
        magma.addScalarAndColor(0.0f, "magma-black");//technically magma doesn't quite reach black at the minimum end (0 0 4), or ~(0 0 2) if extended by one, as it is a 256 element lookup, but it is close enough
        magma.addScalarAndColor(-1.0f, "magma-black");
        addPalette(magma);
    }

    //matplotlib's viridis, approximated by reducing their massive 256-entry lookup table to 32 entries
    //Copyright (c) 2012-2015 Matplotlib Development Team; All Rights Reserved
    //see https://github.com/matplotlib/matplotlib/blob/master/LICENSE/LICENSE or debian/copyright for the matplotlib license, "All Rights Reserved" is required language by the license, but it is actually open source
    if (this->getPaletteByName("viridis") == NULL) {
        Palette viridis;
        viridis.setName("viridis");
        this->addColor("viridis-black", 68, 1, 84);//negative and zero
        this->addColor("viridis-1", 68, 1, 84);
        this->addColor("viridis-2", 70, 12, 95);
        this->addColor("viridis-3", 71, 24, 106);
        this->addColor("viridis-4", 72, 34, 115);
        this->addColor("viridis-5", 70, 45, 124);
        this->addColor("viridis-6", 68, 55, 129);
        this->addColor("viridis-7", 65, 65, 134);
        this->addColor("viridis-8", 61, 74, 137);
        this->addColor("viridis-9", 57, 84, 139);
        this->addColor("viridis-10", 53, 92, 140);
        this->addColor("viridis-11", 49, 100, 141);
        this->addColor("viridis-12", 46, 108, 142);
        this->addColor("viridis-13", 42, 117, 142);
        this->addColor("viridis-14", 39, 124, 142);
        this->addColor("viridis-15", 36, 132, 141);
        this->addColor("viridis-16", 34, 139, 141);
        this->addColor("viridis-17", 31, 148, 139);
        this->addColor("viridis-18", 30, 155, 137);
        this->addColor("viridis-19", 31, 163, 134);
        this->addColor("viridis-20", 36, 170, 130);
        this->addColor("viridis-21", 46, 178, 124);
        this->addColor("viridis-22", 57, 185, 118);
        this->addColor("viridis-23", 71, 192, 110);
        this->addColor("viridis-24", 87, 198, 101);
        this->addColor("viridis-25", 107, 205, 89);
        this->addColor("viridis-26", 126, 210, 78);
        this->addColor("viridis-27", 146, 215, 65);
        this->addColor("viridis-28", 167, 219, 51);
        this->addColor("viridis-29", 191, 223, 36);
        this->addColor("viridis-30", 212, 225, 26);
        this->addColor("viridis-31", 233, 228, 25);
        this->addColor("viridis-32", 253, 231, 36);
        viridis.addScalarAndColor(32.0f / 32.0f, "viridis-32");
        viridis.addScalarAndColor(31.0f / 32.0f, "viridis-31");
        viridis.addScalarAndColor(30.0f / 32.0f, "viridis-30");
        viridis.addScalarAndColor(29.0f / 32.0f, "viridis-29");
        viridis.addScalarAndColor(28.0f / 32.0f, "viridis-28");
        viridis.addScalarAndColor(27.0f / 32.0f, "viridis-27");
        viridis.addScalarAndColor(26.0f / 32.0f, "viridis-26");
        viridis.addScalarAndColor(25.0f / 32.0f, "viridis-25");
        viridis.addScalarAndColor(24.0f / 32.0f, "viridis-24");
        viridis.addScalarAndColor(23.0f / 32.0f, "viridis-23");
        viridis.addScalarAndColor(22.0f / 32.0f, "viridis-22");
        viridis.addScalarAndColor(21.0f / 32.0f, "viridis-21");
        viridis.addScalarAndColor(20.0f / 32.0f, "viridis-20");
        viridis.addScalarAndColor(19.0f / 32.0f, "viridis-19");
        viridis.addScalarAndColor(18.0f / 32.0f, "viridis-18");
        viridis.addScalarAndColor(17.0f / 32.0f, "viridis-17");
        viridis.addScalarAndColor(16.0f / 32.0f, "viridis-16");
        viridis.addScalarAndColor(15.0f / 32.0f, "viridis-15");
        viridis.addScalarAndColor(14.0f / 32.0f, "viridis-14");
        viridis.addScalarAndColor(13.0f / 32.0f, "viridis-13");
        viridis.addScalarAndColor(12.0f / 32.0f, "viridis-12");
        viridis.addScalarAndColor(11.0f / 32.0f, "viridis-11");
        viridis.addScalarAndColor(10.0f / 32.0f, "viridis-10");
        viridis.addScalarAndColor(9.0f / 32.0f, "viridis-9");
        viridis.addScalarAndColor(8.0f / 32.0f, "viridis-8");
        viridis.addScalarAndColor(7.0f / 32.0f, "viridis-7");
        viridis.addScalarAndColor(6.0f / 32.0f, "viridis-6");
        viridis.addScalarAndColor(5.0f / 32.0f, "viridis-5");
        viridis.addScalarAndColor(4.0f / 32.0f, "viridis-4");
        viridis.addScalarAndColor(3.0f / 32.0f, "viridis-3");
        viridis.addScalarAndColor(2.0f / 32.0f, "viridis-2");
        viridis.addScalarAndColor(1.0f / 32.0f, "viridis-1");
        viridis.addScalarAndColor(0.0f, "viridis-black");
        viridis.addScalarAndColor(-1.0f, "viridis-black");
        addPalette(viridis);
    }

    //matplotlib plasma, 32-entry LUT
    //Copyright (c) 2012-2015 Matplotlib Development Team; All Rights Reserved
    if (this->getPaletteByName("plasma") == NULL) {
        Palette plasma;
        plasma.setName("plasma");
        this->addColor("plasma-black", 12, 7, 134);//negative and zero
        this->addColor("plasma-1", 12, 7, 134);
        this->addColor("plasma-2", 33, 5, 143);
        this->addColor("plasma-3", 49, 4, 150);
        this->addColor("plasma-4", 63, 3, 156);
        this->addColor("plasma-5", 78, 2, 161);
        this->addColor("plasma-6", 90, 0, 165);
        this->addColor("plasma-7", 103, 0, 167);
        this->addColor("plasma-8", 115, 0, 168);
        this->addColor("plasma-9", 129, 4, 167);
        this->addColor("plasma-10", 140, 10, 164);
        this->addColor("plasma-11", 151, 19, 160);
        this->addColor("plasma-12", 162, 28, 154);
        this->addColor("plasma-13", 173, 38, 146);
        this->addColor("plasma-14", 182, 47, 139);
        this->addColor("plasma-15", 190, 56, 131);
        this->addColor("plasma-16", 198, 65, 124);
        this->addColor("plasma-17", 207, 75, 116);
        this->addColor("plasma-18", 214, 85, 109);
        this->addColor("plasma-19", 220, 94, 102);
        this->addColor("plasma-20", 227, 103, 95);
        this->addColor("plasma-21", 233, 114, 87);
        this->addColor("plasma-22", 238, 124, 80);
        this->addColor("plasma-23", 243, 134, 73);
        this->addColor("plasma-24", 246, 145, 66);
        this->addColor("plasma-25", 250, 157, 58);
        this->addColor("plasma-26", 252, 169, 52);
        this->addColor("plasma-27", 253, 181, 45);
        this->addColor("plasma-28", 253, 193, 40);
        this->addColor("plasma-29", 251, 208, 36);
        this->addColor("plasma-30", 248, 221, 36);
        this->addColor("plasma-31", 244, 234, 38);
        this->addColor("plasma-32", 239, 248, 33);
        plasma.addScalarAndColor(32.0f / 32.0f, "plasma-32");
        plasma.addScalarAndColor(31.0f / 32.0f, "plasma-31");
        plasma.addScalarAndColor(30.0f / 32.0f, "plasma-30");
        plasma.addScalarAndColor(29.0f / 32.0f, "plasma-29");
        plasma.addScalarAndColor(28.0f / 32.0f, "plasma-28");
        plasma.addScalarAndColor(27.0f / 32.0f, "plasma-27");
        plasma.addScalarAndColor(26.0f / 32.0f, "plasma-26");
        plasma.addScalarAndColor(25.0f / 32.0f, "plasma-25");
        plasma.addScalarAndColor(24.0f / 32.0f, "plasma-24");
        plasma.addScalarAndColor(23.0f / 32.0f, "plasma-23");
        plasma.addScalarAndColor(22.0f / 32.0f, "plasma-22");
        plasma.addScalarAndColor(21.0f / 32.0f, "plasma-21");
        plasma.addScalarAndColor(20.0f / 32.0f, "plasma-20");
        plasma.addScalarAndColor(19.0f / 32.0f, "plasma-19");
        plasma.addScalarAndColor(18.0f / 32.0f, "plasma-18");
        plasma.addScalarAndColor(17.0f / 32.0f, "plasma-17");
        plasma.addScalarAndColor(16.0f / 32.0f, "plasma-16");
        plasma.addScalarAndColor(15.0f / 32.0f, "plasma-15");
        plasma.addScalarAndColor(14.0f / 32.0f, "plasma-14");
        plasma.addScalarAndColor(13.0f / 32.0f, "plasma-13");
        plasma.addScalarAndColor(12.0f / 32.0f, "plasma-12");
        plasma.addScalarAndColor(11.0f / 32.0f, "plasma-11");
        plasma.addScalarAndColor(10.0f / 32.0f, "plasma-10");
        plasma.addScalarAndColor(9.0f / 32.0f, "plasma-9");
        plasma.addScalarAndColor(8.0f / 32.0f, "plasma-8");
        plasma.addScalarAndColor(7.0f / 32.0f, "plasma-7");
        plasma.addScalarAndColor(6.0f / 32.0f, "plasma-6");
        plasma.addScalarAndColor(5.0f / 32.0f, "plasma-5");
        plasma.addScalarAndColor(4.0f / 32.0f, "plasma-4");
        plasma.addScalarAndColor(3.0f / 32.0f, "plasma-3");
        plasma.addScalarAndColor(2.0f / 32.0f, "plasma-2");
        plasma.addScalarAndColor(1.0f / 32.0f, "plasma-1");
        plasma.addScalarAndColor(0.0f, "plasma-black");
        plasma.addScalarAndColor(-1.0f, "plasma-black");
        addPalette(plasma);
    }

    //matplotlib inferno, 32-entry LUT
    //Copyright (c) 2012-2015 Matplotlib Development Team; All Rights Reserved
    if (this->getPaletteByName("inferno") == NULL) {
        Palette inferno;
        inferno.setName("inferno");
        this->addColor("inferno-black", 0, 0, 3);//negative and zero
        this->addColor("inferno-1", 0, 0, 3);
        this->addColor("inferno-2", 3, 2, 18);
        this->addColor("inferno-3", 10, 7, 35);
        this->addColor("inferno-4", 20, 11, 54);
        this->addColor("inferno-5", 34, 11, 76);
        this->addColor("inferno-6", 48, 10, 92);
        this->addColor("inferno-7", 62, 9, 102);
        this->addColor("inferno-8", 75, 12, 107);
        this->addColor("inferno-9", 90, 17, 109);
        this->addColor("inferno-10", 102, 21, 110);
        this->addColor("inferno-11", 115, 26, 109);
        this->addColor("inferno-12", 128, 31, 107);
        this->addColor("inferno-13", 142, 36, 104);
        this->addColor("inferno-14", 155, 40, 100);
        this->addColor("inferno-15", 167, 45, 95);
        this->addColor("inferno-16", 180, 51, 88);
        this->addColor("inferno-17", 193, 58, 80);
        this->addColor("inferno-18", 204, 65, 72);
        this->addColor("inferno-19", 214, 74, 63);
        this->addColor("inferno-20", 223, 84, 54);
        this->addColor("inferno-21", 232, 97, 43);
        this->addColor("inferno-22", 239, 109, 33);
        this->addColor("inferno-23", 244, 122, 22);
        this->addColor("inferno-24", 248, 136, 12);
        this->addColor("inferno-25", 251, 153, 6);
        this->addColor("inferno-26", 251, 168, 13);
        this->addColor("inferno-27", 251, 183, 28);
        this->addColor("inferno-28", 249, 199, 47);
        this->addColor("inferno-29", 245, 217, 72);
        this->addColor("inferno-30", 241, 232, 100);
        this->addColor("inferno-31", 242, 244, 133);
        this->addColor("inferno-32", 252, 254, 164);
        inferno.addScalarAndColor(32.0f / 32.0f, "inferno-32");
        inferno.addScalarAndColor(31.0f / 32.0f, "inferno-31");
        inferno.addScalarAndColor(30.0f / 32.0f, "inferno-30");
        inferno.addScalarAndColor(29.0f / 32.0f, "inferno-29");
        inferno.addScalarAndColor(28.0f / 32.0f, "inferno-28");
        inferno.addScalarAndColor(27.0f / 32.0f, "inferno-27");
        inferno.addScalarAndColor(26.0f / 32.0f, "inferno-26");
        inferno.addScalarAndColor(25.0f / 32.0f, "inferno-25");
        inferno.addScalarAndColor(24.0f / 32.0f, "inferno-24");
        inferno.addScalarAndColor(23.0f / 32.0f, "inferno-23");
        inferno.addScalarAndColor(22.0f / 32.0f, "inferno-22");
        inferno.addScalarAndColor(21.0f / 32.0f, "inferno-21");
        inferno.addScalarAndColor(20.0f / 32.0f, "inferno-20");
        inferno.addScalarAndColor(19.0f / 32.0f, "inferno-19");
        inferno.addScalarAndColor(18.0f / 32.0f, "inferno-18");
        inferno.addScalarAndColor(17.0f / 32.0f, "inferno-17");
        inferno.addScalarAndColor(16.0f / 32.0f, "inferno-16");
        inferno.addScalarAndColor(15.0f / 32.0f, "inferno-15");
        inferno.addScalarAndColor(14.0f / 32.0f, "inferno-14");
        inferno.addScalarAndColor(13.0f / 32.0f, "inferno-13");
        inferno.addScalarAndColor(12.0f / 32.0f, "inferno-12");
        inferno.addScalarAndColor(11.0f / 32.0f, "inferno-11");
        inferno.addScalarAndColor(10.0f / 32.0f, "inferno-10");
        inferno.addScalarAndColor(9.0f / 32.0f, "inferno-9");
        inferno.addScalarAndColor(8.0f / 32.0f, "inferno-8");
        inferno.addScalarAndColor(7.0f / 32.0f, "inferno-7");
        inferno.addScalarAndColor(6.0f / 32.0f, "inferno-6");
        inferno.addScalarAndColor(5.0f / 32.0f, "inferno-5");
        inferno.addScalarAndColor(4.0f / 32.0f, "inferno-4");
        inferno.addScalarAndColor(3.0f / 32.0f, "inferno-3");
        inferno.addScalarAndColor(2.0f / 32.0f, "inferno-2");
        inferno.addScalarAndColor(1.0f / 32.0f, "inferno-1");
        inferno.addScalarAndColor(0.0f, "inferno-black");
        inferno.addScalarAndColor(-1.0f, "inferno-black");
        addPalette(inferno);
    }

    //Crameri berlin diverging colormap (Fabio Crameri scientific colour maps, MIT license)
    if (this->getPaletteByName("berlin") == NULL) {
        Palette berlin;
        berlin.setName("berlin");
        this->addColor("berlin-0", 158, 175, 254);
        this->addColor("berlin-1", 139, 173, 246);
        this->addColor("berlin-2", 120, 171, 237);
        this->addColor("berlin-3", 100, 166, 225);
        this->addColor("berlin-4", 81, 159, 211);
        this->addColor("berlin-5", 64, 147, 192);
        this->addColor("berlin-6", 53, 133, 172);
        this->addColor("berlin-7", 46, 118, 153);
        this->addColor("berlin-8", 39, 103, 133);
        this->addColor("berlin-9", 34, 89, 115);
        this->addColor("berlin-10", 29, 74, 96);
        this->addColor("berlin-11", 23, 61, 79);
        this->addColor("berlin-12", 19, 48, 62);
        this->addColor("berlin-13", 16, 36, 46);
        this->addColor("berlin-14", 16, 25, 31);
        this->addColor("berlin-15", 18, 17, 19);
        this->addColor("berlin-16", 25, 11, 8);
        this->addColor("berlin-17", 33, 11, 2);
        this->addColor("berlin-18", 42, 13, 0);
        this->addColor("berlin-19", 53, 15, 0);
        this->addColor("berlin-20", 64, 18, 0);
        this->addColor("berlin-21", 76, 22, 2);
        this->addColor("berlin-22", 91, 28, 7);
        this->addColor("berlin-23", 107, 38, 17);
        this->addColor("berlin-24", 125, 51, 30);
        this->addColor("berlin-25", 142, 66, 46);
        this->addColor("berlin-26", 158, 81, 63);
        this->addColor("berlin-27", 173, 96, 80);
        this->addColor("berlin-28", 189, 111, 99);
        this->addColor("berlin-29", 206, 126, 117);
        this->addColor("berlin-30", 223, 142, 136);
        this->addColor("berlin-31", 239, 158, 156);
        this->addColor("berlin-32", 254, 173, 173);
        berlin.addScalarAndColor(16.0f / 16.0f, "berlin-32");
        berlin.addScalarAndColor(15.0f / 16.0f, "berlin-31");
        berlin.addScalarAndColor(14.0f / 16.0f, "berlin-30");
        berlin.addScalarAndColor(13.0f / 16.0f, "berlin-29");
        berlin.addScalarAndColor(12.0f / 16.0f, "berlin-28");
        berlin.addScalarAndColor(11.0f / 16.0f, "berlin-27");
        berlin.addScalarAndColor(10.0f / 16.0f, "berlin-26");
        berlin.addScalarAndColor(9.0f / 16.0f, "berlin-25");
        berlin.addScalarAndColor(8.0f / 16.0f, "berlin-24");
        berlin.addScalarAndColor(7.0f / 16.0f, "berlin-23");
        berlin.addScalarAndColor(6.0f / 16.0f, "berlin-22");
        berlin.addScalarAndColor(5.0f / 16.0f, "berlin-21");
        berlin.addScalarAndColor(4.0f / 16.0f, "berlin-20");
        berlin.addScalarAndColor(3.0f / 16.0f, "berlin-19");
        berlin.addScalarAndColor(2.0f / 16.0f, "berlin-18");
        berlin.addScalarAndColor(1.0f / 16.0f, "berlin-17");
        berlin.addScalarAndColor(0.0f, "berlin-16");
        berlin.addScalarAndColor(-1.0f / 16.0f, "berlin-15");
        berlin.addScalarAndColor(-2.0f / 16.0f, "berlin-14");
        berlin.addScalarAndColor(-3.0f / 16.0f, "berlin-13");
        berlin.addScalarAndColor(-4.0f / 16.0f, "berlin-12");
        berlin.addScalarAndColor(-5.0f / 16.0f, "berlin-11");
        berlin.addScalarAndColor(-6.0f / 16.0f, "berlin-10");
        berlin.addScalarAndColor(-7.0f / 16.0f, "berlin-9");
        berlin.addScalarAndColor(-8.0f / 16.0f, "berlin-8");
        berlin.addScalarAndColor(-9.0f / 16.0f, "berlin-7");
        berlin.addScalarAndColor(-10.0f / 16.0f, "berlin-6");
        berlin.addScalarAndColor(-11.0f / 16.0f, "berlin-5");
        berlin.addScalarAndColor(-12.0f / 16.0f, "berlin-4");
        berlin.addScalarAndColor(-13.0f / 16.0f, "berlin-3");
        berlin.addScalarAndColor(-14.0f / 16.0f, "berlin-2");
        berlin.addScalarAndColor(-15.0f / 16.0f, "berlin-1");
        berlin.addScalarAndColor(-16.0f / 16.0f, "berlin-0");
        addPalette(berlin);
    }

    //Crameri managua diverging colormap (Fabio Crameri scientific colour maps, MIT license)
    if (this->getPaletteByName("managua") == NULL) {
        Palette managua;
        managua.setName("managua");
        this->addColor("managua-0", 255, 207, 103);
        this->addColor("managua-1", 245, 191, 97);
        this->addColor("managua-2", 235, 175, 92);
        this->addColor("managua-3", 225, 160, 87);
        this->addColor("managua-4", 215, 146, 82);
        this->addColor("managua-5", 206, 133, 78);
        this->addColor("managua-6", 196, 121, 74);
        this->addColor("managua-7", 186, 109, 70);
        this->addColor("managua-8", 176, 97, 66);
        this->addColor("managua-9", 165, 86, 63);
        this->addColor("managua-10", 153, 76, 60);
        this->addColor("managua-11", 141, 65, 58);
        this->addColor("managua-12", 128, 56, 57);
        this->addColor("managua-13", 116, 48, 57);
        this->addColor("managua-14", 104, 43, 59);
        this->addColor("managua-15", 94, 40, 64);
        this->addColor("managua-16", 86, 41, 73);
        this->addColor("managua-17", 81, 44, 84);
        this->addColor("managua-18", 77, 51, 98);
        this->addColor("managua-19", 75, 60, 113);
        this->addColor("managua-20", 75, 70, 128);
        this->addColor("managua-21", 77, 82, 143);
        this->addColor("managua-22", 80, 94, 157);
        this->addColor("managua-23", 83, 106, 169);
        this->addColor("managua-24", 87, 118, 180);
        this->addColor("managua-25", 92, 131, 190);
        this->addColor("managua-26", 97, 143, 200);
        this->addColor("managua-27", 102, 157, 209);
        this->addColor("managua-28", 107, 171, 218);
        this->addColor("managua-29", 112, 186, 227);
        this->addColor("managua-30", 118, 201, 237);
        this->addColor("managua-31", 123, 217, 246);
        this->addColor("managua-32", 128, 231, 254);
        managua.addScalarAndColor(16.0f / 16.0f, "managua-32");
        managua.addScalarAndColor(15.0f / 16.0f, "managua-31");
        managua.addScalarAndColor(14.0f / 16.0f, "managua-30");
        managua.addScalarAndColor(13.0f / 16.0f, "managua-29");
        managua.addScalarAndColor(12.0f / 16.0f, "managua-28");
        managua.addScalarAndColor(11.0f / 16.0f, "managua-27");
        managua.addScalarAndColor(10.0f / 16.0f, "managua-26");
        managua.addScalarAndColor(9.0f / 16.0f, "managua-25");
        managua.addScalarAndColor(8.0f / 16.0f, "managua-24");
        managua.addScalarAndColor(7.0f / 16.0f, "managua-23");
        managua.addScalarAndColor(6.0f / 16.0f, "managua-22");
        managua.addScalarAndColor(5.0f / 16.0f, "managua-21");
        managua.addScalarAndColor(4.0f / 16.0f, "managua-20");
        managua.addScalarAndColor(3.0f / 16.0f, "managua-19");
        managua.addScalarAndColor(2.0f / 16.0f, "managua-18");
        managua.addScalarAndColor(1.0f / 16.0f, "managua-17");
        managua.addScalarAndColor(0.0f, "managua-16");
        managua.addScalarAndColor(-1.0f / 16.0f, "managua-15");
        managua.addScalarAndColor(-2.0f / 16.0f, "managua-14");
        managua.addScalarAndColor(-3.0f / 16.0f, "managua-13");
        managua.addScalarAndColor(-4.0f / 16.0f, "managua-12");
        managua.addScalarAndColor(-5.0f / 16.0f, "managua-11");
        managua.addScalarAndColor(-6.0f / 16.0f, "managua-10");
        managua.addScalarAndColor(-7.0f / 16.0f, "managua-9");
        managua.addScalarAndColor(-8.0f / 16.0f, "managua-8");
        managua.addScalarAndColor(-9.0f / 16.0f, "managua-7");
        managua.addScalarAndColor(-10.0f / 16.0f, "managua-6");
        managua.addScalarAndColor(-11.0f / 16.0f, "managua-5");
        managua.addScalarAndColor(-12.0f / 16.0f, "managua-4");
        managua.addScalarAndColor(-13.0f / 16.0f, "managua-3");
        managua.addScalarAndColor(-14.0f / 16.0f, "managua-2");
        managua.addScalarAndColor(-15.0f / 16.0f, "managua-1");
        managua.addScalarAndColor(-16.0f / 16.0f, "managua-0");
        addPalette(managua);
    }

    //matplotlib jet (classic blue-cyan-green-yellow-red), 32-entry LUT
    //Copyright (c) 2012-2015 Matplotlib Development Team; All Rights Reserved
    if (this->getPaletteByName("jet") == NULL) {
        Palette jet;
        jet.setName("jet");
        this->addColor("jet-black", 0, 0, 127);//low / negative
        this->addColor("jet-1", 0, 0, 127);
        this->addColor("jet-2", 0, 0, 163);
        this->addColor("jet-3", 0, 0, 200);
        this->addColor("jet-4", 0, 0, 236);
        this->addColor("jet-5", 0, 4, 255);
        this->addColor("jet-6", 0, 36, 255);
        this->addColor("jet-7", 0, 68, 255);
        this->addColor("jet-8", 0, 100, 255);
        this->addColor("jet-9", 0, 136, 255);
        this->addColor("jet-10", 0, 168, 255);
        this->addColor("jet-11", 0, 200, 255);
        this->addColor("jet-12", 2, 232, 244);
        this->addColor("jet-13", 31, 255, 215);
        this->addColor("jet-14", 57, 255, 189);
        this->addColor("jet-15", 83, 255, 163);
        this->addColor("jet-16", 108, 255, 137);
        this->addColor("jet-17", 137, 255, 108);
        this->addColor("jet-18", 163, 255, 83);
        this->addColor("jet-19", 189, 255, 57);
        this->addColor("jet-20", 215, 255, 31);
        this->addColor("jet-21", 244, 248, 2);
        this->addColor("jet-22", 255, 218, 0);
        this->addColor("jet-23", 255, 189, 0);
        this->addColor("jet-24", 255, 159, 0);
        this->addColor("jet-25", 255, 126, 0);
        this->addColor("jet-26", 255, 96, 0);
        this->addColor("jet-27", 255, 66, 0);
        this->addColor("jet-28", 255, 37, 0);
        this->addColor("jet-29", 236, 3, 0);
        this->addColor("jet-30", 200, 0, 0);
        this->addColor("jet-31", 163, 0, 0);
        this->addColor("jet-32", 127, 0, 0);
        jet.addScalarAndColor(32.0f / 32.0f, "jet-32");
        jet.addScalarAndColor(31.0f / 32.0f, "jet-31");
        jet.addScalarAndColor(30.0f / 32.0f, "jet-30");
        jet.addScalarAndColor(29.0f / 32.0f, "jet-29");
        jet.addScalarAndColor(28.0f / 32.0f, "jet-28");
        jet.addScalarAndColor(27.0f / 32.0f, "jet-27");
        jet.addScalarAndColor(26.0f / 32.0f, "jet-26");
        jet.addScalarAndColor(25.0f / 32.0f, "jet-25");
        jet.addScalarAndColor(24.0f / 32.0f, "jet-24");
        jet.addScalarAndColor(23.0f / 32.0f, "jet-23");
        jet.addScalarAndColor(22.0f / 32.0f, "jet-22");
        jet.addScalarAndColor(21.0f / 32.0f, "jet-21");
        jet.addScalarAndColor(20.0f / 32.0f, "jet-20");
        jet.addScalarAndColor(19.0f / 32.0f, "jet-19");
        jet.addScalarAndColor(18.0f / 32.0f, "jet-18");
        jet.addScalarAndColor(17.0f / 32.0f, "jet-17");
        jet.addScalarAndColor(16.0f / 32.0f, "jet-16");
        jet.addScalarAndColor(15.0f / 32.0f, "jet-15");
        jet.addScalarAndColor(14.0f / 32.0f, "jet-14");
        jet.addScalarAndColor(13.0f / 32.0f, "jet-13");
        jet.addScalarAndColor(12.0f / 32.0f, "jet-12");
        jet.addScalarAndColor(11.0f / 32.0f, "jet-11");
        jet.addScalarAndColor(10.0f / 32.0f, "jet-10");
        jet.addScalarAndColor(9.0f / 32.0f, "jet-9");
        jet.addScalarAndColor(8.0f / 32.0f, "jet-8");
        jet.addScalarAndColor(7.0f / 32.0f, "jet-7");
        jet.addScalarAndColor(6.0f / 32.0f, "jet-6");
        jet.addScalarAndColor(5.0f / 32.0f, "jet-5");
        jet.addScalarAndColor(4.0f / 32.0f, "jet-4");
        jet.addScalarAndColor(3.0f / 32.0f, "jet-3");
        jet.addScalarAndColor(2.0f / 32.0f, "jet-2");
        jet.addScalarAndColor(1.0f / 32.0f, "jet-1");
        jet.addScalarAndColor(0.0f, "jet-black");
        jet.addScalarAndColor(-1.0f, "jet-black");
        addPalette(jet);
    }


    if (this->getPaletteByName("JET256") == NULL) {
        Palette JET256;
        JET256.setName("JET256");

        //summary of original slow "lookup table" (if closer to previous implementation is desired):
        //start: 0 -> (0 0 132)
        //change: 0.121 -> (0 0 255)
        //change: 0.372 -> (0 255 255)
        //change: 0.623 -> (255 255 0)
        //change: 0.874 -> (255 0 0)
        //end: 1 -> (127 0 0)
        
        //alternative round-valued version via https://gist.github.com/bagrow/805122
        /*(0 0.0 0.0 0.5, \
           1 0.0 0.0 1.0, \
           2 0.0 0.5 1.0, \ <- redundant
           3 0.0 1.0 1.0, \
           4 0.5 1.0 0.5, \ <- redundant
           5 1.0 1.0 0.0, \
           6 1.0 0.5 0.0, \ <- redundant
           7 1.0 0.0 0.0, \
           8 0.5 0.0 0.0 )*/
        
        this->addColor("_J0", 0, 0, 127);//rounding to probably-intended colors
        this->addColor("_J1", 0, 0, 255);
        this->addColor("_J3", 0, 255, 255);//skipping redundant points
        this->addColor("_J5", 255, 255, 0);
        this->addColor("_J7", 255, 0, 0);
        this->addColor("_J8", 127, 0, 0);
        
        JET256.addScalarAndColor(1.0f, "_J8");
        JET256.addScalarAndColor(0.875f, "_J7");//also rounding to probably-intended control points
        JET256.addScalarAndColor(0.625f, "_J5");
        JET256.addScalarAndColor(0.375f, "_J3");
        JET256.addScalarAndColor(0.125f, "_J1");
        JET256.addScalarAndColor(0.0f, "_J0");
        
        addPalette(JET256);
    }
    
    //TSC: palettes that use "none" or are broken (psych no none) start here - exception: rbgyr20p, to keep it close to rbgyr20
    //----------------------------------------------------------------------
    // Psych palette
    //
    if (this->getPaletteByName("PSYCH") == NULL) {
        /*this->addColor("_pyell-oran",  0xff, 0xcc, 0x00 );//don't add colors repeatedly, since GiftiLabelTable now warns about everything
        this->addColor("_poran-red",  0xff, 0x44, 0x00 );
        this->addColor("_pblue",  0x00, 0x44, 0xff );
        this->addColor("_pltblue1",  0x00, 0x69, 0xff );
        this->addColor("_pltblue2",  0x00, 0x99, 0xff );
        this->addColor("_pbluecyan",  0x00, 0xcc, 0xff );//*/
        
        Palette psych;
        psych.setName("PSYCH");
        //psych.setPositiveOnly(false);
        
        psych.addScalarAndColor(1.00f, "_yellow");
        psych.addScalarAndColor(0.75f, "_pyell-oran");
        psych.addScalarAndColor(0.50f, "_orange");
        psych.addScalarAndColor(0.25f, "_poran-red");
        psych.addScalarAndColor(0.05f, "none");
        psych.addScalarAndColor(-0.05f, "_pblue");
        psych.addScalarAndColor(-0.25f, "_pltblue1");
        psych.addScalarAndColor(-0.50f, "_pltblue2");
        psych.addScalarAndColor(-0.75f, "_pbluecyan");
        
        addPalette(psych);
    }
    //----------------------------------------------------------------------
    // Psych no-none palette
    //
    if (this->getPaletteByName("PSYCH-NO-NONE") == NULL) {
        /*this->addColor("_pyell-oran",  0xff, 0xcc, 0x00 );
        this->addColor("_poran-red",  0xff, 0x44, 0x00 );
        this->addColor("_pblue",  0x00, 0x44, 0xff );
        this->addColor("_pltblue1",  0x00, 0x69, 0xff );
        this->addColor("_pltblue2",  0x00, 0x99, 0xff );
        this->addColor("_pbluecyan",  0x00, 0xcc, 0xff );//*/
        
        Palette psychNoNone;
        psychNoNone.setName("PSYCH-NO-NONE");
        //psychNoNone.setPositiveOnly(false);
        
        psychNoNone.addScalarAndColor(1.00f, "_yellow");
        psychNoNone.addScalarAndColor(0.75f, "_pyell-oran");
        psychNoNone.addScalarAndColor(0.50f, "_orange");
        psychNoNone.addScalarAndColor(0.25f, "_poran-red");
        psychNoNone.addScalarAndColor(0.0f, "_pblue");
        psychNoNone.addScalarAndColor(-0.25f, "_pltblue1");
        psychNoNone.addScalarAndColor(-0.50f, "_pltblue2");
        psychNoNone.addScalarAndColor(-0.75f, "_pbluecyan");
        
        addPalette(psychNoNone);
    }
    
    //----------------------------------------------------------------------
    // ROY-BIG palette
    //
    if (this->getPaletteByName("ROY-BIG") == NULL) {
        Palette royBig;
        royBig.setName("ROY-BIG");
        
        royBig.addScalarAndColor(1.00f, "_RGB_255_255_0");
        royBig.addScalarAndColor(0.875f, "_RGB_255_200_0");
        royBig.addScalarAndColor(0.750f, "_RGB_255_120_0");
        royBig.addScalarAndColor(0.625f, "_RGB_255_0_0");
        royBig.addScalarAndColor(0.500f, "_RGB_200_0_0");
        royBig.addScalarAndColor(0.375f, "_RGB_150_0_0");
        royBig.addScalarAndColor(0.250f, "_RGB_100_0_0");
        royBig.addScalarAndColor(0.125f, "_RGB_60_0_0");
        royBig.addScalarAndColor(0.000f, "none");
        royBig.addScalarAndColor(-0.125f, "_RGB_0_0_80");
        royBig.addScalarAndColor(-0.250f, "_RGB_0_0_170");
        royBig.addScalarAndColor(-0.375f, "_RGB_75_0_125");
        royBig.addScalarAndColor(-0.500f, "_RGB_125_0_160");
        royBig.addScalarAndColor(-0.625f, "_RGB_75_125_0");
        royBig.addScalarAndColor(-0.750f, "_RGB_0_200_0");
        royBig.addScalarAndColor(-0.875f, "_RGB_0_255_0");
        royBig.addScalarAndColor(-0.990f, "_RGB_0_255_255");
        royBig.addScalarAndColor(-1.00f, "_RGB_0_255_255");
        
        addPalette(royBig);
    }
    
    if (this->getPaletteByName("clear_brain") == NULL) {
        Palette clearBrain;
        clearBrain.setName("clear_brain");
        clearBrain.addScalarAndColor(1.0f , "_red");
        clearBrain.addScalarAndColor(0.9f , "_orange");
        clearBrain.addScalarAndColor(0.8f , "_oran-yell");
        clearBrain.addScalarAndColor(0.7f , "_yellow");
        clearBrain.addScalarAndColor(0.6f , "_limegreen");
        clearBrain.addScalarAndColor(0.5f , "_green");
        clearBrain.addScalarAndColor(0.4f , "_blue_videen7");
        clearBrain.addScalarAndColor(0.3f , "_blue_videen9");
        clearBrain.addScalarAndColor(0.2f , "_blue_videen11");
        clearBrain.addScalarAndColor(0.1f , "_purple2");
        clearBrain.addScalarAndColor(0.0f , "none");
        clearBrain.addScalarAndColor(-0.1f , "_cyan");
        clearBrain.addScalarAndColor(-0.2f , "_green");
        clearBrain.addScalarAndColor(-0.3f , "_limegreen");
        clearBrain.addScalarAndColor(-0.4f , "_violet");
        clearBrain.addScalarAndColor(-0.5f , "_hotpink");
        clearBrain.addScalarAndColor(-0.6f , "_white");
        clearBrain.addScalarAndColor(-0.7f , "_gry-dd");
        clearBrain.addScalarAndColor(-0.8f , "_gry-bb");
        clearBrain.addScalarAndColor(-0.9f , "_black");
        addPalette(clearBrain);
    }
    
    if (this->getPaletteByName("fidl") == NULL) {
        int Bright_Yellow[3] = { 0xee, 0xee, 0x55 };
        this->addColor("_Bright_Yellow", Bright_Yellow);
        int Mustard[3] = { 0xdd, 0xdd, 0x66 };
        this->addColor("_Mustard", Mustard);
        int Brown_Mustard[3] = { 0xdd, 0x99, 0x00 };
        this->addColor("_Brown_Mustard", Brown_Mustard);
        int Bright_Red[3] = { 0xff, 0x00, 0x00 };
        this->addColor("_Bright_Red", Bright_Red);
        int Fire_Engine_Red[3] = { 0xdd, 0x00, 0x00 };
        this->addColor("_Fire_Engine_Red", Fire_Engine_Red);
        int Brick[3] = { 0xbb, 0x00, 0x00 };
        this->addColor("_Brick", Brick);
        int Beet[3] = { 0x99, 0x00, 0x00 };
        this->addColor("_Beet", Beet);
        int Beaujolais[3] = { 0x77, 0x00, 0x00 };
        this->addColor("_Beaujolais", Beaujolais);
        int Burgundy[3] = { 0x55, 0x00, 0x00 };
        this->addColor("_Burgundy", Burgundy);
        int Thrombin[3] = { 0x11, 0x00, 0x00 };
        this->addColor("_Thrombin", Thrombin);
        int Deep_Green[3] = { 0x00, 0x11, 0x00 };
        this->addColor("_Deep_Green", Deep_Green);
        int British_Racing_Green[3] = { 0x00, 0x55, 0x00 };
        this->addColor("_British_Racing_Green", British_Racing_Green);
        int Kelp[3] = { 0x00, 0x77, 0x00 };
        this->addColor("_Kelp", Kelp);
        int Lime[3] = { 0x00, 0x99, 0x00 };
        this->addColor("_Lime", Lime);
        int Mint[3] = { 0x00, 0xbb, 0x00 };
        this->addColor("_Mint", Mint);
        int Brussell_Sprout[3] = { 0x00, 0xdd, 0x00 };
        this->addColor("_Brussell_Sprout", Brussell_Sprout);
        int Bright_Green[3] = { 0x00, 0xff, 0x00 };
        this->addColor("_Bright_Green", Bright_Green);
        int Periwinkle[3] = { 0x66, 0x66, 0xbb };
        this->addColor("_Periwinkle", Periwinkle);
        int Azure[3] = { 0x88, 0x88, 0xee };
        this->addColor("_Azure", Azure);
        int Turquoise[3] = { 0x00, 0xcc, 0xcc };
        this->addColor("_Turquoise", Turquoise);
        
        Palette fidl;
        fidl.setName("fidl");
        fidl.addScalarAndColor(1.0f, "_Bright_Yellow");
        fidl.addScalarAndColor(0.9f, "_Mustard");
        fidl.addScalarAndColor(0.8f, "_Brown_Mustard");
        fidl.addScalarAndColor(0.7f, "_Bright_Red");
        fidl.addScalarAndColor(0.6f, "_Fire_Engine_Red");
        fidl.addScalarAndColor(0.5f, "_Brick");
        fidl.addScalarAndColor(0.4f, "_Beet");
        fidl.addScalarAndColor(0.3f, "_Beaujolais");
        fidl.addScalarAndColor(0.2f, "_Burgundy");
        fidl.addScalarAndColor(0.1f, "_Thrombin");
        fidl.addScalarAndColor(0.0f, "none");
        fidl.addScalarAndColor(-0.1f, "_Deep_Green");
        fidl.addScalarAndColor(-0.2f, "_British_Racing_Green");
        fidl.addScalarAndColor(-0.3f, "_Kelp");
        fidl.addScalarAndColor(-0.4f, "_Lime");
        fidl.addScalarAndColor(-0.5f, "_Mint");
        fidl.addScalarAndColor(-0.6f, "_Brussell_Sprout");
        fidl.addScalarAndColor(-0.7f, "_Bright_Green");
        fidl.addScalarAndColor(-0.8f, "_Periwinkle");
        fidl.addScalarAndColor(-0.9f, "_Azure");
        fidl.addScalarAndColor(-1.0f, "_Turquoise");
        addPalette(fidl);
    }
    
    //------------------------------------------------------------------------
    //
    // Colors by Russ H.
    //
    /*int _rbgyr20_10[3] = { 0x00, 0xff, 0x00 };
    this->addColor("_rbgyr20_10", _rbgyr20_10);
    int _rbgyr20_15[3] = { 0xff, 0xff, 0x00 };
    this->addColor("_rbgyr20_15", _rbgyr20_15);
    int _rbgyr20_20[3] = { 0xff, 0x00, 0x00 };
    this->addColor("_rbgyr20_20", _rbgyr20_20);//*/
    
    int _rbgyr20_21[3] = { 0x9d, 0x22, 0xc1 };
    this->addColor("_rbgyr20_21", _rbgyr20_21);
    int _rbgyr20_22[3] = { 0x81, 0x06, 0xa5 };
    this->addColor("_rbgyr20_22", _rbgyr20_22);
    int _rbgyr20_23[3] = { 0xff, 0xec, 0x00 };
    this->addColor("_rbgyr20_23", _rbgyr20_23);
    int _rbgyr20_24[3] = { 0xff, 0xd6, 0x00 };
    this->addColor("_rbgyr20_24", _rbgyr20_24);
    int _rbgyr20_25[3] = { 0xff, 0xbc, 0x00 };
    this->addColor("_rbgyr20_25", _rbgyr20_25);
    int _rbgyr20_26[3] = { 0xff, 0x9c, 0x00 };
    this->addColor("_rbgyr20_26", _rbgyr20_26);
    int _rbgyr20_27[3] = { 0xff, 0x7c, 0x00 };
    this->addColor("_rbgyr20_27", _rbgyr20_27);
    int _rbgyr20_28[3] = { 0xff, 0x5c, 0x00 };
    this->addColor("_rbgyr20_28", _rbgyr20_28);
    int _rbgyr20_29[3] = { 0xff, 0x3d, 0x00 };
    this->addColor("_rbgyr20_29", _rbgyr20_29);
    int _rbgyr20_30[3] = { 0xff, 0x23, 0x00 };
    this->addColor("_rbgyr20_30", _rbgyr20_30);
    int _rbgyr20_31[3] = { 0x00, 0xed, 0x12 };
    this->addColor("_rbgyr20_31", _rbgyr20_31);
    int _rbgyr20_32[3] = { 0x00, 0xd5, 0x2a };
    this->addColor("_rbgyr20_32", _rbgyr20_32);
    int _rbgyr20_33[3] = { 0x00, 0xb9, 0x46 };
    this->addColor("_rbgyr20_33", _rbgyr20_33);
    int _rbgyr20_34[3] = { 0x00, 0x9b, 0x64 };
    this->addColor("_rbgyr20_34", _rbgyr20_34);
    int _rbgyr20_35[3] = { 0x00, 0x7b, 0x84 };
    this->addColor("_rbgyr20_35", _rbgyr20_35);
    int _rbgyr20_36[3] = { 0x00, 0x5b, 0xa4 };
    this->addColor("_rbgyr20_36", _rbgyr20_36);
    int _rbgyr20_37[3] = { 0x00, 0x44, 0xbb };
    this->addColor("_rbgyr20_37", _rbgyr20_37);
    int _rbgyr20_38[3] = { 0x00, 0x24, 0xdb };
    this->addColor("_rbgyr20_38", _rbgyr20_38);
    int _rbgyr20_39[3] = { 0x00, 0x00, 0xff };
    this->addColor("_rbgyr20_39", _rbgyr20_39);
    
    int _rbgyr20_40[3] = { 0xff, 0xf1, 0x00 };
    this->addColor("_rbgyr20_40", _rbgyr20_40);
    int _rbgyr20_41[3] = { 0xff, 0xdc, 0x00 };
    this->addColor("_rbgyr20_41", _rbgyr20_41);
    int _rbgyr20_42[3] = { 0xff, 0xcb, 0x00 };
    this->addColor("_rbgyr20_42", _rbgyr20_42);
    int _rbgyr20_43[3] = { 0xff, 0xc2, 0x00 };
    this->addColor("_rbgyr20_43", _rbgyr20_43);
    int _rbgyr20_44[3] = { 0xff, 0xae, 0x00 };
    this->addColor("_rbgyr20_44", _rbgyr20_44);
    int _rbgyr20_45[3] = { 0xff, 0x9f, 0x00 };
    this->addColor("_rbgyr20_45", _rbgyr20_45);
    int _rbgyr20_46[3] = { 0xff, 0x86, 0x00 };
    this->addColor("_rbgyr20_46", _rbgyr20_46);
    int _rbgyr20_47[3] = { 0xff, 0x59, 0x00 };
    this->addColor("_rbgyr20_47", _rbgyr20_47);
    int _rbgyr20_48[3] = { 0x00, 0xff, 0x2d };
    this->addColor("_rbgyr20_48", _rbgyr20_48);
    int _rbgyr20_49[3] = { 0x00, 0xff, 0x65 };
    this->addColor("_rbgyr20_49", _rbgyr20_49);
    int _rbgyr20_50[3] = { 0x00, 0xff, 0xa5 };
    this->addColor("_rbgyr20_50", _rbgyr20_50);
    int _rbgyr20_51[3] = { 0x00, 0xff, 0xdd };
    this->addColor("_rbgyr20_51", _rbgyr20_51);
    int _rbgyr20_52[3] = { 0x00, 0xff, 0xff };
    this->addColor("_rbgyr20_52", _rbgyr20_52);
    int _rbgyr20_53[3] = { 0x00, 0xe9, 0xff };
    this->addColor("_rbgyr20_53", _rbgyr20_53);
    int _rbgyr20_54[3] = { 0x00, 0xad, 0xff };
    this->addColor("_rbgyr20_54", _rbgyr20_54);
    int _rbgyr20_55[3] = { 0x00, 0x69, 0xff };
    this->addColor("_rbgyr20_55", _rbgyr20_55);
    int _rbgyr20_56[3] = { 0xff, 0x00, 0xb9 };
    this->addColor("_rbgyr20_56", _rbgyr20_56);
    int _rbgyr20_57[3] = { 0xff, 0x00, 0x63 };
    this->addColor("_rbgyr20_57", _rbgyr20_57);
    int _rbgyr20_58[3] = { 0xff, 0x05, 0x00 };
    this->addColor("_rbgyr20_58", _rbgyr20_58);
    int _rbgyr20_59[3] = { 0xff, 0x32, 0x00 };
    this->addColor("_rbgyr20_59", _rbgyr20_59);
    int _rbgyr20_60[3] =  { 0xff, 0x70, 0x00 };
    this->addColor("_rbgyr20_60", _rbgyr20_60);
    int _rbgyr20_61[3] = { 0xff, 0xa4, 0x00 };
    this->addColor("_rbgyr20_61", _rbgyr20_61);
    int _rbgyr20_62[3] = { 0xff, 0xba, 0x00 };
    this->addColor("_rbgyr20_62", _rbgyr20_62);
    int _rbgyr20_63[3] = { 0xff, 0xd3, 0x00 };
    this->addColor("_rbgyr20_63", _rbgyr20_63);
    int _rbgyr20_64[3] = { 0x42, 0x21, 0xdb };
    this->addColor("_rbgyr20_64", _rbgyr20_64);
    int _rbgyr20_65[3] = { 0x10, 0x08, 0xf6 };
    this->addColor("_rbgyr20_65", _rbgyr20_65);
    int _rbgyr20_66[3] = { 0x00, 0x13, 0xff };
    this->addColor("_rbgyr20_66", _rbgyr20_66);
    int _rbgyr20_67[3] = { 0x00, 0x5b, 0xff };
    this->addColor("_rbgyr20_67", _rbgyr20_67);
    int _rbgyr20_68[3] = { 0x00, 0xb3, 0xff };
    this->addColor("_rbgyr20_68", _rbgyr20_68);
    int _rbgyr20_69[3] = { 0x00, 0xfc, 0xff };
    this->addColor("_rbgyr20_69", _rbgyr20_69);
    int _rbgyr20_70[3] = { 0x00, 0xff, 0xcd };
    this->addColor("_rbgyr20_70", _rbgyr20_70);
    int _rbgyr20_71[3] = { 0x00, 0xff, 0x74 };
    this->addColor("_rbgyr20_71", _rbgyr20_71);
    int _rbgyr20_72[3] = { 0xff, 0x00, 0xf9 };
    this->addColor("_rbgyr20_72", _rbgyr20_72);
    int _rbgyr20_73[3] = { 0x62, 0x31, 0xc9 };
    this->addColor("_rbgyr20_73", _rbgyr20_73);
    
    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("raich4_clrmid") == NULL) {
        Palette r4;
        r4.setName("raich4_clrmid");
        r4.addScalarAndColor(1.000000f, "_rbgyr20_20");
        r4.addScalarAndColor(0.900000f, "_rbgyr20_30");
        r4.addScalarAndColor(0.800000f, "_rbgyr20_29");
        r4.addScalarAndColor(0.700000f, "_rbgyr20_28");
        r4.addScalarAndColor(0.600000f, "_rbgyr20_27");
        r4.addScalarAndColor(0.500000f, "_rbgyr20_26");
        r4.addScalarAndColor(0.400000f, "_rbgyr20_25");
        r4.addScalarAndColor(0.300000f, "_rbgyr20_24");
        r4.addScalarAndColor(0.200000f, "_rbgyr20_23");
        r4.addScalarAndColor(0.100000f, "_rbgyr20_15");
        r4.addScalarAndColor(0.000000f, "none");
        r4.addScalarAndColor(-0.100000f, "_rbgyr20_10");
        r4.addScalarAndColor(-0.200000f, "_rbgyr20_31");
        r4.addScalarAndColor(-0.300000f, "_rbgyr20_32");
        r4.addScalarAndColor(-0.400000f, "_rbgyr20_33");
        r4.addScalarAndColor(-0.500000f, "_rbgyr20_34");
        r4.addScalarAndColor(-0.600000f, "_rbgyr20_35");
        r4.addScalarAndColor(-0.700000f, "_rbgyr20_36");
        r4.addScalarAndColor(-0.800000f, "_rbgyr20_37");
        r4.addScalarAndColor(-0.900000f, "_rbgyr20_38");
        r4.addScalarAndColor(-1.000000f, "_rbgyr20_39");
        addPalette(r4);
    }
    
    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("raich6_clrmid") == NULL) {
        Palette r6;
        r6.setName("raich6_clrmid");
        r6.addScalarAndColor(1.000000f, "_rbgyr20_20");
        r6.addScalarAndColor(0.900000f, "_rbgyr20_47");
        r6.addScalarAndColor(0.800000f, "_rbgyr20_46");
        r6.addScalarAndColor(0.700000f, "_rbgyr20_45");
        r6.addScalarAndColor(0.600000f, "_rbgyr20_44");
        r6.addScalarAndColor(0.500000f, "_rbgyr20_43");
        r6.addScalarAndColor(0.400000f, "_rbgyr20_42");
        r6.addScalarAndColor(0.300000f, "_rbgyr20_41");
        r6.addScalarAndColor(0.200000f, "_rbgyr20_40");
        r6.addScalarAndColor(0.100000f, "_rbgyr20_15");
        r6.addScalarAndColor(0.000000f, "none");
        r6.addScalarAndColor(-0.100000f, "_rbgyr20_10");
        r6.addScalarAndColor(-0.200000f, "_rbgyr20_48");
        r6.addScalarAndColor(-0.300000f, "_rbgyr20_49");
        r6.addScalarAndColor(-0.400000f, "_rbgyr20_50");
        r6.addScalarAndColor(-0.500000f, "_rbgyr20_51");
        r6.addScalarAndColor(-0.600000f, "_rbgyr20_52");
        r6.addScalarAndColor(-0.700000f, "_rbgyr20_53");
        r6.addScalarAndColor(-0.800000f, "_rbgyr20_54");
        r6.addScalarAndColor(-0.900000f, "_rbgyr20_55");
        r6.addScalarAndColor(-1.000000f, "_rbgyr20_39");
        addPalette(r6);
    }
    
    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("HSB8_clrmid") == NULL) {
        Palette hsb8;
        hsb8.setName("HSB8_clrmid");
        hsb8.addScalarAndColor(1.000000f, "_rbgyr20_15");
        hsb8.addScalarAndColor(0.900000f, "_rbgyr20_63");
        hsb8.addScalarAndColor(0.800000f, "_rbgyr20_62");
        hsb8.addScalarAndColor(0.700000f, "_rbgyr20_61");
        hsb8.addScalarAndColor(0.600000f, "_rbgyr20_60");
        hsb8.addScalarAndColor(0.500000f, "_rbgyr20_59");
        hsb8.addScalarAndColor(0.400000f, "_rbgyr20_58");
        hsb8.addScalarAndColor(0.300000f, "_rbgyr20_57");
        hsb8.addScalarAndColor(0.200000f, "_rbgyr20_56");
        hsb8.addScalarAndColor(0.100000f, "_rbgyr20_72");
        hsb8.addScalarAndColor(0.000000f, "none");
        hsb8.addScalarAndColor(-0.100000f, "_rbgyr20_73");
        hsb8.addScalarAndColor(-0.200000f, "_rbgyr20_64");
        hsb8.addScalarAndColor(-0.300000f, "_rbgyr20_65");
        hsb8.addScalarAndColor(-0.400000f, "_rbgyr20_66");
        hsb8.addScalarAndColor(-0.500000f, "_rbgyr20_67");
        hsb8.addScalarAndColor(-0.600000f, "_rbgyr20_68");
        hsb8.addScalarAndColor(-0.700000f, "_rbgyr20_69");
        hsb8.addScalarAndColor(-0.800000f, "_rbgyr20_70");
        hsb8.addScalarAndColor(-0.900000f, "_rbgyr20_71");
        hsb8.addScalarAndColor(-1.000000f, "_rbgyr20_10");
        addPalette(hsb8);
    }
    
    //----------------------------------------------------------------------
    // Positive/Negative palette
    //
    if (this->getPaletteByName("POS_NEG") == NULL) {
        /*this->addColor("pos_neg_blue",  0x00, 0x00, 0xff );
        this->addColor("pos_neg_red",  0xff, 0x00, 0x00 );//*/
        
        Palette posNeg;
        posNeg.setName("POS_NEG");
        
        posNeg.addScalarAndColor(1.00f, "pos_neg_red");
        posNeg.addScalarAndColor(0.0001f, "none");
        posNeg.addScalarAndColor(-0.0001f, "pos_neg_blue");
        
        addPalette(posNeg);
    }
    
    /*
     * Special palette name that actually maps the
     * volume's first three maps an RGB volume
     */
    if (this->getPaletteByName(Palette::SPECIAL_RGB_VOLUME_PALETTE_NAME) == NULL) {
        Palette rgbVolumePalette;
        rgbVolumePalette.setName(Palette::SPECIAL_RGB_VOLUME_PALETTE_NAME);
        
        const AString redName("RGB_Volume_RED");
        const AString greenName("RGB_Volume_GREEN");
        const AString blueName("RGB_Volume_BLUE");
        
        this->addColor(redName,  255,   0,   0);
        this->addColor(greenName,  0, 255,   0);
        this->addColor(blueName,   0,   0, 255);
        
        rgbVolumePalette.addScalarAndColor(1.0f,    blueName);
        rgbVolumePalette.addScalarAndColor(0.333f,  greenName);
        rgbVolumePalette.addScalarAndColor(-0.333f, redName);
        
        addPalette(rgbVolumePalette);
    }

    // Nilearn colormaps from nilearn/plotting/cm.py.
    if (this->getPaletteByName("black_red") == NULL) {
        Palette blackRed;
        blackRed.setName("black_red");
        addPaletteScalarAndColor(blackRed,  1.0f,      255, 255, 255);
        addPaletteScalarAndColor(blackRed,  0.746032f, 255, 255,   0);
        addPaletteScalarAndColor(blackRed,  0.365079f, 255,   0,   0);
        addPaletteScalarAndColor(blackRed,  0.0f,       11,   0,   0);
        addPaletteScalarAndColor(blackRed, -1.0f,       11,   0,   0);
        addPalette(blackRed);
    }

    if (this->getPaletteByName("cold_white_hot") == NULL) {
        Palette coldWhiteHot;
        coldWhiteHot.setName("cold_white_hot");
        addPaletteScalarAndColor(coldWhiteHot,  1.0f,       11,   0,   0);
        addPaletteScalarAndColor(coldWhiteHot,  0.634921f, 255,   0,   0);
        addPaletteScalarAndColor(coldWhiteHot,  0.253968f, 255, 255,   0);
        addPaletteScalarAndColor(coldWhiteHot,  0.0f,      255, 255, 255);
        addPaletteScalarAndColor(coldWhiteHot, -0.253968f,   0, 255, 255);
        addPaletteScalarAndColor(coldWhiteHot, -0.634921f,   0,   0, 255);
        addPaletteScalarAndColor(coldWhiteHot, -1.0f,        0,   0,  11);
        addPalette(coldWhiteHot);
    }

    if (this->getPaletteByName("black_green") == NULL) {
        Palette blackGreen;
        blackGreen.setName("black_green");
        addPaletteScalarAndColor(blackGreen,  1.0f,      255, 255, 255);
        addPaletteScalarAndColor(blackGreen,  0.746032f, 255, 255,   0);
        addPaletteScalarAndColor(blackGreen,  0.365079f,   0, 255,   0);
        addPaletteScalarAndColor(blackGreen,  0.0f,        0,  11,   0);
        addPaletteScalarAndColor(blackGreen, -1.0f,        0,  11,   0);
        addPalette(blackGreen);
    }

    if (this->getPaletteByName("black_blue") == NULL) {
        Palette blackBlue;
        blackBlue.setName("black_blue");
        addPaletteScalarAndColor(blackBlue,  1.0f,      255, 255, 255);
        addPaletteScalarAndColor(blackBlue,  0.746032f,   0, 255, 255);
        addPaletteScalarAndColor(blackBlue,  0.365079f,   0,   0, 255);
        addPaletteScalarAndColor(blackBlue,  0.0f,        0,   0,  11);
        addPaletteScalarAndColor(blackBlue, -1.0f,        0,   0,  11);
        addPalette(blackBlue);
    }

    if (this->getPaletteByName("hot_white_bone") == NULL) {
        Palette hotWhiteBone;
        hotWhiteBone.setName("hot_white_bone");
        addPaletteScalarAndColor(hotWhiteBone,  1.0f,        0,   0,   0);
        addPaletteScalarAndColor(hotWhiteBone,  0.817461f,  81,  81, 113);
        addPaletteScalarAndColor(hotWhiteBone,  0.626984f, 166, 198, 198);
        addPaletteScalarAndColor(hotWhiteBone,  0.5f,      255, 255, 255);
        addPaletteScalarAndColor(hotWhiteBone,  0.444444f, 255, 255, 198);
        addPaletteScalarAndColor(hotWhiteBone,  0.388889f, 255, 255, 142);
        addPaletteScalarAndColor(hotWhiteBone,  0.333333f, 255, 212,  85);
        addPaletteScalarAndColor(hotWhiteBone,  0.277778f, 255, 156,  28);
        addPaletteScalarAndColor(hotWhiteBone,  0.222222f, 227,  99,   0);
        addPaletteScalarAndColor(hotWhiteBone,  0.166667f, 170,  43,   0);
        addPaletteScalarAndColor(hotWhiteBone,  0.111111f, 113,   0,   0);
        addPaletteScalarAndColor(hotWhiteBone,  0.055556f,  57,   0,   0);
        addPaletteScalarAndColor(hotWhiteBone,  0.0f,        0,   0,   0);
        addPaletteScalarAndColor(hotWhiteBone, -1.0f,        0,   0,   0);
        addPalette(hotWhiteBone);
    }

    if (this->getPaletteByName("ocean_hot") == NULL) {
        Palette oceanHot;
        oceanHot.setName("ocean_hot");
        addPaletteScalarAndColor(oceanHot,  1.0f,        11,   0,   0);
        addPaletteScalarAndColor(oceanHot,  0.817461f, 255,   0,   0);
        addPaletteScalarAndColor(oceanHot,  0.626984f, 255, 255,   0);
        addPaletteScalarAndColor(oceanHot,  0.5f,      255, 255, 255);
        addPaletteScalarAndColor(oceanHot,  0.444444f, 170, 212, 227);
        addPaletteScalarAndColor(oceanHot,  0.388889f,  85, 170, 198);
        addPaletteScalarAndColor(oceanHot,  0.333333f,   0, 127, 170);
        addPaletteScalarAndColor(oceanHot,  0.277778f,   0,  85, 142);
        addPaletteScalarAndColor(oceanHot,  0.222222f,   0,  42, 113);
        addPaletteScalarAndColor(oceanHot,  0.166667f,   0,   0,  85);
        addPaletteScalarAndColor(oceanHot,  0.111111f,   0,  43,  57);
        addPaletteScalarAndColor(oceanHot,  0.055556f,   0,  85,  28);
        addPaletteScalarAndColor(oceanHot,  0.0f,        0, 128,   0);
        addPaletteScalarAndColor(oceanHot, -1.0f,        0, 128,   0);
        addPalette(oceanHot);
    }

    // Softer ends than cold_white_hot: dark red / dark blue instead of near-black.
    if (this->getPaletteByName("cold_white_hot_soft") == NULL) {
        Palette coldWhiteHotSoft;
        coldWhiteHotSoft.setName("cold_white_hot_soft");
        addPaletteScalarAndColor(coldWhiteHotSoft,  1.0f,      180,   0,   0);
        addPaletteScalarAndColor(coldWhiteHotSoft,  0.634921f, 255,   0,   0);
        addPaletteScalarAndColor(coldWhiteHotSoft,  0.253968f, 255, 255,   0);
        addPaletteScalarAndColor(coldWhiteHotSoft,  0.0f,      255, 255, 255);
        addPaletteScalarAndColor(coldWhiteHotSoft, -0.253968f,   0, 255, 255);
        addPaletteScalarAndColor(coldWhiteHotSoft, -0.634921f,   0,   0, 255);
        addPaletteScalarAndColor(coldWhiteHotSoft, -1.0f,        0,  60, 180);
        addPalette(coldWhiteHotSoft);
    }

    // Softer ends than hot_white_bone: dark gray / dark red instead of black.
    if (this->getPaletteByName("hot_white_bone_soft") == NULL) {
        Palette hotWhiteBoneSoft;
        hotWhiteBoneSoft.setName("hot_white_bone_soft");
        addPaletteScalarAndColor(hotWhiteBoneSoft,  1.0f,       90,  90, 110);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.817461f,  81,  81, 113);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.626984f, 166, 198, 198);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.5f,      255, 255, 255);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.444444f, 255, 255, 198);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.388889f, 255, 255, 142);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.333333f, 255, 212,  85);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.277778f, 255, 156,  28);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.222222f, 227,  99,   0);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.166667f, 170,  43,   0);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.111111f, 113,   0,   0);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.055556f,  57,   0,   0);
        addPaletteScalarAndColor(hotWhiteBoneSoft,  0.0f,      100,  25,   0);
        addPaletteScalarAndColor(hotWhiteBoneSoft, -1.0f,      120,  35,   0);
        addPalette(hotWhiteBoneSoft);
    }
    
    if (modifiedStatus == false) {
        this->clearModified();//this clears modified status on all members, to match our status
    }
    
    static bool printAllPalettesFlag = false;
    if (printAllPalettesFlag) {
        printAllPalettesFlag = false;
        for (const auto p : this->palettes) {
            std::cout << "----------------------" << std::endl;
            std::cout << p->toString() << std::endl;
            std::cout << "INVERTED" << std::endl;
            std::cout << p->getInvertedPalette()->toString() << std::endl;
            std::cout << "SEPARATE SIGN" << std::endl;
            std::cout << p->getSignSeparateInvertedPalette()->toString() << std::endl;
            std::cout << "SEPARATE NONE";
            if (p->getSignSeparateInvertedPalette() == p->getNoneSeparateInvertedPalette()) {
                std::cout << " same as SEPARATE SIGN" << std::endl;
            }
            else {
                std::cout << std::endl;
                std::cout << p->getNoneSeparateInvertedPalette()->toString() << std::endl;
            }
        }
    }
}

/**
 * @return The structure for this file.
 */
StructureEnum::Enum 
PaletteFile::getStructure() const
{
    // palette files do not have structure
    return StructureEnum::INVALID;
}

/**
 * Set the structure for this file.
 * @param structure
 *   New structure for this file.
 */
void 
PaletteFile::setStructure(const StructureEnum::Enum /*structure*/)
{
    // palette files do not have structure
}

/**
 * @return Get access to the file's metadata.
 */
GiftiMetaData* 
PaletteFile::getFileMetaData()
{
    return this->metadata;
}

/**
 * @return Get access to unmodifiable file's metadata.
 */
const GiftiMetaData* 
PaletteFile::getFileMetaData() const
{
    return this->metadata;
}

/**
 * Set the palette mapping based upon the given file type,
 * file name, data name, and data.
 *
 * @param paletteColorMapping
 *    Palette color mapping that is setup.
 * @param dataFileType
 *    Type of data file.
 * @param fileName
 *    Name of file.
 * @param dataName
 *    Name of data.
 * @param data
 *    The data.
 * @param numberOfDataElements
 *    Number of elements in data.
 */
void
PaletteFile::setDefaultPaletteColorMapping(PaletteColorMapping* paletteColorMapping,
                                           const DataFileTypeEnum::Enum& dataFileType,
                                           const AString& fileNameIn,
                                           const AString& dataNameIn,
                                           const float* data,
                                           const int32_t numberOfDataElements)
{
    bool isShapeCurvatureData = false;
    bool isShapeDepthData = false;
    bool isShapeData = false;
    bool isVolumeAnatomyData = false;
    
    const AString fileName = fileNameIn.toLower();
    const AString dataName = dataNameIn.toLower();
    
    bool invalid = false;
    bool checkShapeFile = false;
    bool checkVolume = false;
    switch (dataFileType) {
        case DataFileTypeEnum::ANNOTATION:
            invalid = true;
            break;
        case DataFileTypeEnum::ANNOTATION_TEXT_SUBSTITUTION:
            invalid = true;
            break;
        case DataFileTypeEnum::BORDER:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_DYNAMIC:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_LABEL:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_PARCEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_DENSE:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_DYNAMIC:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_LABEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_SCALAR:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_SERIES:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_SCALAR:
            checkShapeFile = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_SPARSE:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_TIME_SERIES:
            break;
        case DataFileTypeEnum::CONNECTIVITY_FIBER_ORIENTATIONS_TEMPORARY:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_FIBER_TRAJECTORY_TEMPORARY:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_FIBER_TRAJECTORY_MAPS:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_SCALAR_DATA_SERIES:
            break;
        case DataFileTypeEnum::CZI_IMAGE_FILE:
            invalid = true;
            break;
        case DataFileTypeEnum::FOCI:
            invalid = true;
            break;
        case DataFileTypeEnum::HISTOLOGY_SLICES:
            invalid = true;
            break;
        case DataFileTypeEnum::IMAGE:
            invalid = true;
            break;
        case DataFileTypeEnum::LABEL:
            invalid = true;
            break;
        case DataFileTypeEnum::META_VOLUME:
            checkVolume = true;
            break;
        case DataFileTypeEnum::METRIC:
            checkShapeFile = true;
            break;
        case DataFileTypeEnum::METRIC_DYNAMIC:
            break;
        case DataFileTypeEnum::OME_ZARR_IMAGE_FILE:
            invalid = true;
            break;
        case DataFileTypeEnum::PALETTE:
            invalid = true;
            break;
        case DataFileTypeEnum::RGBA:
            invalid = true;
            break;
        case DataFileTypeEnum::SAMPLES:
            invalid = true;
            break;
        case DataFileTypeEnum::SCENE:
            invalid = true;
            break;
        case DataFileTypeEnum::SPECIFICATION:
            invalid = true;
            break;
        case DataFileTypeEnum::SURFACE:
            invalid = true;
            break;
        case DataFileTypeEnum::UNKNOWN:
            invalid = true;
            break;
        case DataFileTypeEnum::VOLUME:
            checkVolume = true;
            break;
        case DataFileTypeEnum::VOLUME_DYNAMIC:
            break;
    }
    
    if (invalid) {
        return;
    }
    
    if (checkShapeFile) {
        if (dataName.contains("curv")) {
            isShapeData = true;
            isShapeCurvatureData = true;
        }
        else if (dataName.contains("depth")) {
            isShapeData = true;
            isShapeDepthData = true;
        }
        else if (dataName.contains("shape")) {
            isShapeData = true;
        }
        else if (fileName.contains("curv")) {
            isShapeData = true;
            isShapeCurvatureData = true;
        }
        else if (fileName.contains("depth")) {
            isShapeData = true;
            isShapeDepthData = true;
        }
        else if (fileName.contains("shape")) {
            isShapeData = true;
        }
    }
    
    float minValue = std::numeric_limits<float>::max();
    float maxValue = -minValue;
    
    for (int32_t i = 0; i < numberOfDataElements; i++) {
        const float d = data[i];
        if (d > maxValue) {
            maxValue = d;
        }
        if (d < minValue) {
            minValue = d;
        }
    }
    //bool havePositiveData = (maxValue > 0);//unused, commenting out to prevent compiler warning
    bool haveNegativeData = (minValue < 0);
    
    if (checkVolume) {
        if ((minValue >= 0)
            && (maxValue <= 255.0)) {
            isVolumeAnatomyData = true;
        }
    }
    
    if (isVolumeAnatomyData) {
        paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
        paletteColorMapping->setSelectedPaletteName("Gray_Interp_Positive");
        paletteColorMapping->setInterpolatePaletteFlag(true);
        paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
        paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
        paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
        paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
        paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
    }
    else if (isShapeData) {
        paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
        paletteColorMapping->setSelectedPaletteName("Gray_Interp");
        paletteColorMapping->setInterpolatePaletteFlag(true);
        if (isShapeDepthData) {
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_USER_SCALE);
            paletteColorMapping->setUserScaleNegativeMaximum(-30.0);
            paletteColorMapping->setUserScaleNegativeMinimum(0.0);
            paletteColorMapping->setUserScalePositiveMinimum(0.0);
            paletteColorMapping->setUserScalePositiveMaximum(10.0);
        }
        else if (isShapeCurvatureData) {
//            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_USER_SCALE);
//            paletteColorMapping->setUserScaleNegativeMaximum(-1.5);
//            paletteColorMapping->setUserScaleNegativeMinimum(0.0);
//            paletteColorMapping->setUserScalePositiveMinimum(0.0);
//            paletteColorMapping->setUserScalePositiveMaximum(1.5);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_ABSOLUTE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
        }
        else {
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_ABSOLUTE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
        }
        paletteColorMapping->setDisplayNegativeDataFlag(true);
        paletteColorMapping->setDisplayPositiveDataFlag(true);
        paletteColorMapping->setDisplayZeroDataFlag(true);
    }
    else {
        if (haveNegativeData) {
            paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
            paletteColorMapping->setSelectedPaletteName("videen-style");
            paletteColorMapping->setSelectedPaletteName("ROY-BIG-BL");
            paletteColorMapping->setInterpolatePaletteFlag(true);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_ABSOLUTE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
            paletteColorMapping->setDisplayNegativeDataFlag(true);
            paletteColorMapping->setDisplayPositiveDataFlag(true);
            paletteColorMapping->setDisplayZeroDataFlag(false);
        }
        else {
            paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
            paletteColorMapping->setSelectedPaletteName("videen-style");
            paletteColorMapping->setSelectedPaletteName("ROY-BIG-BL");
            paletteColorMapping->setInterpolatePaletteFlag(true);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_ABSOLUTE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
            paletteColorMapping->setDisplayNegativeDataFlag(true);
            paletteColorMapping->setDisplayPositiveDataFlag(true);
            paletteColorMapping->setDisplayZeroDataFlag(false);
        }
    }
    paletteColorMapping->clearModified();
}

