#include "font.h" 

#include <algorithm>

#include "utils/core.h"
#include "utils/fileio.h"
#include "header.h"
#include "utf/utf.hpp"

#define MSDFGEN_PUBLIC
#include "msdfgen.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace devils_engine {
namespace visage {
const font_t::glyph_t *font_t::find_glyph(const uint32_t codepoint) const {
  auto itr = std::upper_bound(glyphs2.begin(), glyphs2.end(), codepoint, [] (const uint32_t codepoint, const auto &g) { return g.codepoint = codepoint; });
  if (itr == glyphs2.end()) return fallback;
  return &(*itr);
}

void font_t::query_font_glyph(float font_height, struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint) const {
  auto g = find_glyph(codepoint);

  const double local_scale = font_height / scale;

  glyph->width = double(g->w) * local_scale; // local_scale ?
  glyph->height = double(g->h) * local_scale;
  glyph->xadvance = g->advance * scale * local_scale;
  glyph->uv[0].x = g->al / double(width);
  glyph->uv[0].y = g->ab / double(height);
  glyph->uv[1].x = g->ar / double(width);
  glyph->uv[1].y = g->at / double(height);
  glyph->offset.x = g->pl * scale * local_scale;
  glyph->offset.y = g->pb * scale * local_scale;
}

double font_t::text_width(double height, const std::string_view &txt) const {
  uint32_t rune = 0;
  size_t size = 0;
  size_t gsize = 0;
  double text_width = 0.0;
  const double local_scale = height / scale;

  gsize = size = nk_utf_decode(txt.data(), &rune, txt.size());
  if (gsize == 0) return 0.0;

  while (size <= txt.size() && gsize != 0 && rune != NK_UTF_INVALID) {
    const auto g = find_glyph(rune);
    text_width += g->advance * scale * local_scale;
    gsize = nk_utf_decode(txt.data() + size, &rune, txt.size() - size);
    size += gsize;
  }

  return text_width;
}

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
  // после этого нужно будет вернуть font_t для каждого фонта отдельно
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
    g.advance = glyph.getAdvance();
    glyph.getBoxRect(g.x,g.y,g.w,g.h);
    g.scale = glyph.getBoxScale();
    g.codepoint = glyph.getCodepoint();
    g.gscale = glyph.getGeometryScale();
    g.index = glyph.getIndex();
    glyph.getQuadAtlasBounds(g.al,g.ab,g.ar,g.at);
    glyph.getQuadPlaneBounds(g.pl,g.pb,g.pr,g.pt); // все еще не доконца понятно что делать с этим
  }

  std::sort(f->glyphs2.begin(), f->glyphs2.end(), [] (const auto &a, const auto &b) {
    return a.codepoint < b.codepoint;
  });

  // переводим данные в удобный для нас вид
  for (auto &g : f->glyphs2) {
    const int ny = f->height - (g.y + g.h);
    const double nab = double(f->height) - g.at;
    const double nat = double(ny + g.h) - 0.5;
    const double npb = 1.0 - g.pt; // все еще немного сомневаюсь на счет этого
    const double npt = 1.0 - g.pb;

    g.y = ny;
    g.ab = nab;
    g.at = nat;
    g.pb = npb;
    g.pt = npt;
  }

  return f;
}
}
}