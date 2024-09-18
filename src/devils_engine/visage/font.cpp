#include "font.h" 

#include "utils/core.h"
#include "utils/fileio.h"
#define MSDFGEN_PUBLIC
#include "msdfgen.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace devils_engine {
namespace visage {
std::unique_ptr<font_t> load_font(const std::string &path) {
  const auto file = file_io::read<uint8_t>(path);

  auto ft = msdfgen::initializeFreetype();
  if (ft == nullptr) utils::error("Could not init freetype");

  auto font = msdfgen::loadFontData(ft, file.data(), file.size());
  if (font == nullptr) utils::error("Could not load font '{}'", path);

  std::vector<msdf_atlas::GlyphGeometry> glyphs;
  msdf_atlas::FontGeometry fontGeometry(&glyphs);

  msdf_atlas::Charset set; // ???

  // мы можем наверное составить чарсеты сразу из нескольких фонтов
  fontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);

  const double max_corner_angle = 3.0; // ???
  for (auto &glyph : glyphs) {
    glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, max_corner_angle, 0);
  }

  msdf_atlas::TightAtlasPacker packer;
  packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);
  packer.setMinimumScale(24.0); // ??? (размеры бокса для глифа?)
  packer.setPixelRange(2.0); // ??? (похоже что делает переходы более плавными)
  packer.setMiterLimit(1.0); // ???
  packer.pack(glyphs.data(), glyphs.size());
  int width = 0, height = 0;
  packer.getDimensions(width, height);
  msdf_atlas::ImmediateAtlasGenerator<
    float, // pixel type of buffer for individual glyphs depends on generator function
    3,     // number of atlas color channels
    msdf_atlas::msdfGenerator, // function to generate bitmaps for individual glyphs
    msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3> // storage
  > generator(width, height);

  msdf_atlas::GeneratorAttributes attributes; // ???
  generator.setAttributes(attributes);
  generator.setThreadCount(4);
  generator.generate(glyphs.data(), glyphs.size());
  auto atlas_storage = generator.atlasStorage();
  // + glyphs

  msdfgen::destroyFont(font);
  msdfgen::deinitializeFreetype(ft);

  // atlas_storage мы должны загрузить в контейнер картинок
  // glyphs мы должны пихнуть в шрифт

  msdfgen::savePng(atlas_storage, "font.png");

  auto f = std::make_unique<font_t>();
  f->width = width;
  f->height = height;
  f->scale = packer.getScale();
  f->metrics.em_size = fontGeometry.getMetrics().emSize;
  f->metrics.ascender_y = fontGeometry.getMetrics().ascenderY;
  f->metrics.descender_y = fontGeometry.getMetrics().descenderY;
  f->metrics.line_height = fontGeometry.getMetrics().lineHeight;
  f->metrics.underline_y = fontGeometry.getMetrics().underlineY;
  f->metrics.underline_thickness = fontGeometry.getMetrics().underlineThickness;
  f->glyphs2.reserve(glyphs.size());
  for (const auto &glyph : glyphs) {
    f->glyphs2.emplace_back();
    auto &g = f->glyphs2.back();
    g.xadvance = glyph.getAdvance();
    glyph.getBoxRect(g.x,g.y,g.w,g.h);
    g.scale = glyph.getBoxScale();
    g.codepoint = glyph.getCodepoint();
    g.gscale = glyph.getGeometryScale();
    g.index = glyph.getIndex();
    glyph.getQuadAtlasBounds(g.al,g.ab,g.ar,g.at);
    glyph.getQuadPlaneBounds(g.pl,g.pb,g.pr,g.pt);
  }

  std::sort(f->glyphs2.begin(), f->glyphs2.end(), [] (const auto &a, const auto &b) {
    return a.codepoint < b.codepoint;
  });

  return f;
}
}
}