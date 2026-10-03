//! The ground the app hands the renderer (A11.1, A11.5): until α02b makes areas from cells, the demo area (A5.3),
//! its materials shown as the catalogue's surfaces; from α02a, coarse ground's tiles of 8 × 8 cells, made from the
//! cells' coarse ground (A5.5) as the view needs them.
//!
//! Implements PRE-20, WLD-12 and PRE-03 in part, see A11.5, A5.3 and A5.5: each square metre of ground is drawn as
//! its surface, and the land beyond as coarse ground.

use kd_core::geo::{CellIx, TICKS_PER_M};
use kd_core::time::GameTime;
use kd_data::Catalogue;
use kd_view::{AreaMeshes, CoarseTile, TILE_CELLS, TILE_RING, TILE_SIDE, WaterPiece};
use kd_world::area::{Ground, Material};
use kd_world::cells::{COARSE_SIDE, CellCtx, NONE, coarse_ground};
use kd_world::plants::{PlantGroup, density};

/// The surface each of the demo's materials shows, by id, until α02a's rocks and soils name their own: soil grows
/// grass, and both kinds of bed show bare rock (T01b.3).
const DEMO_SURFACES: [(Material, &str); 5] = [
    (Material::Soil, "grass"),
    (Material::Dirt, "dirt"),
    (Material::Scree, "scree"),
    (Material::HardRock, "rock"),
    (Material::SoftRock, "rock"),
];

/// Each material's surface number, indexed by the material's byte, or the id the catalogue lacks.
pub fn surface_numbers(cat: &Catalogue) -> Result<[u8; Material::ALL.len()], String> {
    let mut out = [0u8; Material::ALL.len()];
    for (m, id) in DEMO_SURFACES {
        let s = cat
            .surface(id)
            .ok_or_else(|| format!("the catalogue has no surface {id:?}"))?;
        out[m as usize] = u8::try_from(s.number).map_err(|_| format!("surface {id:?} is numbered over 255"))?;
    }
    Ok(out)
}

/// An area's ground as the renderer takes it (A11.1): heights in metres above its lowest point, and each square's
/// surface number from its material.
pub fn area_meshes(g: &Ground, surfaces: &[u8; Material::ALL.len()]) -> AreaMeshes {
    AreaMeshes {
        id: g.id,
        base_m: g.base_dm as f32 / 10.0,
        heights: g.heights.iter().map(|&h| f32::from(h) / 10.0).collect(),
        surfaces: g.material.iter().map(|&m| surfaces[usize::from(m)]).collect(),
    }
}

/// A tile of coarse ground (A11.5): the coarse ground of its 8 × 8 cells, their edges shared, in one grid of 257 ×
/// 257 points 32 m apart; each square's surface from its north-west point's material; where the sea lies; the cover
/// of its cells and the ring round them, by plant density (A5.5), with the surfaces their biome shows each group as;
/// and its water lines, in pieces from point to point. Cells beyond the land, void or outside the world's rows, show
/// as bare soil.
pub fn coarse_tile(cx: &CellCtx, tile: CellIx, surfaces: &[u8; Material::ALL.len()], date: GameTime) -> CoarseTile {
    let (tx, ty) = tile.xy();
    let n = TILE_SIDE;
    let step = COARSE_SIDE - 1;
    let mut heights = vec![0.0f32; n * n];
    let mut material = vec![Material::Soil; n * n];
    let mut sea = vec![0u8; n * n];
    let mut water = Vec::new();
    let origin = tile.origin();
    let soil = surfaces[Material::Soil as usize];
    for k in 0..TILE_CELLS * TILE_CELLS {
        let c = CellIx::at(tx + k % TILE_CELLS, ty + k / TILE_CELLS);
        let g = coarse_ground(cx, c);
        let (ox, oy) = ((k % TILE_CELLS) as usize * step, (k / TILE_CELLS) as usize * step);
        for j in 0..COARSE_SIDE {
            for i in 0..COARSE_SIDE {
                let at = (oy + j) * n + ox + i;
                let from = j * COARSE_SIDE + i;
                heights[at] = g.heights[from];
                material[at] = g.material[from];
                sea[at] = u8::from(g.water[from].is_some());
            }
        }
        for line in &g.lines {
            let local = |p: [i32; 2]| {
                [
                    (p[0] - origin.x) as f32 / TICKS_PER_M as f32,
                    (p[1] - origin.y) as f32 / TICKS_PER_M as f32,
                ]
            };
            for (k, w) in line.points.windows(2).enumerate() {
                let (a, b) = (local(w[0]), local(w[1]));
                water.push(WaterPiece {
                    from: [a[0], a[1], line.z_m[k]],
                    to: [b[0], b[1], line.z_m[k + 1]],
                    half_width_m: line.width_m / 2.0,
                    depth_m: line.depth_m,
                    river: line.river,
                    drainage_km2: line.drainage_km2,
                });
            }
        }
    }
    let base_m = heights.iter().copied().fold(f32::INFINITY, f32::min);
    for h in &mut heights {
        *h -= base_m;
    }
    for p in &mut water {
        p.from[2] -= base_m;
        p.to[2] -= base_m;
    }
    let squares = n - 1;
    let surfaces_of = (0..squares * squares)
        .map(|k| surfaces[material[(k / squares) * n + k % squares] as usize])
        .collect();
    // The cells' cover, the tile's and the ring round it.
    let f = &cx.cells.fixed;
    let ring = TILE_RING as i64;
    let mut cover = Vec::with_capacity(TILE_RING * TILE_RING);
    let mut cover_surfaces = Vec::with_capacity(TILE_RING * TILE_RING);
    for k in 0..ring * ring {
        let (x, y) = (i64::from(tx) + k % ring - 1, i64::from(ty) + k / ring - 1);
        let rows = i64::from(kd_core::geo::CELLS_Y);
        let biome = (0..rows).contains(&y).then(|| {
            let c = CellIx::at(x.rem_euclid(i64::from(kd_core::geo::CELLS_X)) as u32, y as u32);
            let b = f.biome[c.0 as usize];
            (c, cx.cat.biomes.iter().find(|e| b != NONE && e.number == u16::from(b)))
        });
        match biome {
            Some((c, Some(b))) => {
                let share = |g: PlantGroup| (density(&cx.cells.state, c, g, date) * 255.0).round() as u8;
                let groups = PlantGroup::ALL.map(share);
                let used: u16 = groups.iter().map(|&v| u16::from(v)).sum();
                let bare = 255u16.saturating_sub(used) as u8;
                cover.push([groups[0], groups[1], groups[2], groups[3], bare]);
                cover_surfaces.push(b.surfaces.map(|s| s as u8));
            }
            _ => {
                cover.push([0, 0, 0, 0, 255]);
                cover_surfaces.push([soil; 5]);
            }
        }
    }
    CoarseTile {
        cell: tile,
        base_m,
        heights,
        surfaces: surfaces_of,
        soil_surface: soil,
        sea,
        cover,
        cover_surfaces,
        water,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-03 WLD-12 PRE-26 WLD-34
    #[test]
    fn coarse_tiles_from_the_island() {
        // The tile holding the first camp and its neighbour east: their shared edge the same to the bit; the
        // escarpment's rim showing rock and its foot scree; the woods' cover by density, shown as the biome says;
        // the river's pieces drawn, every piece's ends in metres from the tile's corner.
        let cat = Catalogue::load(crate::CATALOGUE).expect("the embedded catalogue");
        let w = kd_world::lands::build(cat.land(crate::FIRST_LAND).unwrap(), crate::ISLAND_SEED, &cat).unwrap();
        let cx = CellCtx {
            seed: crate::ISLAND_SEED,
            cells: &w.cells,
            cat: &cat,
        };
        let numbers = surface_numbers(&cat).unwrap();
        let camp = w.cells.fixed.cave_records.iter().find(|r| r.first_camp).unwrap().cell;
        let (x, y) = camp.xy();
        let id = CellIx::at(x - x % TILE_CELLS, y - y % TILE_CELLS);
        let t = coarse_tile(&cx, id, &numbers, GameTime(0));
        let east = coarse_tile(
            &cx,
            CellIx::at(x - x % TILE_CELLS + TILE_CELLS, y - y % TILE_CELLS),
            &numbers,
            GameTime(0),
        );
        let n = TILE_SIDE;
        for j in 0..n {
            let (a, b) = (t.heights[j * n + n - 1] + t.base_m, east.heights[j * n] + east.base_m);
            assert!((a - b).abs() < 1e-3, "row {j}: {a} {b}");
        }
        let count = |m: Material| t.surfaces.iter().filter(|&&s| s == numbers[m as usize]).count();
        // The face runs some 3 km through this tile, one 32 m square deep, beside the valley's break.
        assert!(count(Material::SoftRock) > 50 && count(Material::Scree) == count(Material::SoftRock));
        assert!(count(Material::Soil) > t.surfaces.len() * 9 / 10);
        assert_eq!(t.soil_surface, numbers[Material::Soil as usize]);
        // The camp's cell, in the ring: the island's cover, by density, shown as its biome says.
        let (cx_, cy_) = ((x % TILE_CELLS + 1) as usize, (y % TILE_CELLS + 1) as usize);
        let k = cy_ * TILE_RING + cx_;
        let st = &w.cells.state.cover[camp.0 as usize];
        assert_eq!(t.cover[k], *st);
        let biome = cat
            .biomes
            .iter()
            .find(|b| b.number == u16::from(w.cells.fixed.biome[camp.0 as usize]))
            .unwrap();
        assert_eq!(t.cover_surfaces[k], biome.surfaces.map(|s| s as u8));
        // The river runs through the camp's tile in pieces of at most 32 m, starting where the last one ended.
        let river: Vec<&WaterPiece> = t.water.iter().filter(|p| p.river).collect();
        assert!(river.len() > 30, "{} river pieces", river.len());
        for p in &river {
            let len = ((p.to[0] - p.from[0]).powi(2) + (p.to[1] - p.from[1]).powi(2)).sqrt();
            assert!(len <= 32.5, "{len}");
            assert!(p.half_width_m == 9.0 && p.depth_m > 0.0);
        }
        assert!(
            t.water
                .iter()
                .all(|p| p.from.iter().chain(&p.to).all(|v| v.is_finite()))
        );
        // A tile some 50 km south of the island's middle, out at sea: all sea, no lines; and one in the void
        // beyond, no sea.
        let (mx, my) = w.centre.xy();
        let tile_at = |x: u32, y: u32| CellIx::at(x - x % TILE_CELLS, y - y % TILE_CELLS);
        let sea = coarse_tile(&cx, tile_at(mx, my + 48), &numbers, GameTime(0));
        assert!(sea.sea.iter().all(|&s| s == 1) && sea.water.is_empty());
        let void = coarse_tile(&cx, tile_at(w.window.x0, w.window.y0), &numbers, GameTime(0));
        assert!(void.sea.iter().all(|&s| s == 0));
    }

    // checks: PRE-20 WLD-12
    #[test]
    fn demo_materials_show_surfaces() {
        let cat = Catalogue::load(crate::CATALOGUE).expect("the embedded catalogue");
        let numbers = surface_numbers(&cat).expect("every material has its surface");
        let id = |m: Material| {
            let n = numbers[m as usize];
            cat.surfaces
                .iter()
                .find(|s| s.number == u16::from(n))
                .map(|s| s.id.as_str())
        };
        assert_eq!(id(Material::Soil), Some("grass"));
        assert_eq!(id(Material::Dirt), Some("dirt"));
        assert_eq!(id(Material::Scree), Some("scree"));
        assert_eq!(
            (id(Material::HardRock), id(Material::SoftRock)),
            (Some("rock"), Some("rock"))
        );
        // Every material is mapped once, and rock draws as rock.
        assert!(
            Material::ALL
                .iter()
                .all(|m| DEMO_SURFACES.iter().filter(|d| d.0 == *m).count() == 1)
        );
        assert!(cat.surface("rock").is_some_and(|r| r.rock));
    }
}
