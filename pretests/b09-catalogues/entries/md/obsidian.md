# Obsidian

Ingredient · matter layer 1 · checks `RCK-01`

Natural volcanic glass, mostly silica, formed when lava cools too fast to grow crystals. Like all glass it breaks with shell-like (conchoidal) fractures, giving the sharpest edges of any stone.

| Property | Value | Src |
|---|---|---|
| Density | about 2.4 g/cm³ | 1 |
| Hardness (Mohs) | 5.5 Mohs (5–6) | 1 |
| Young's modulus | 70 GPa ± 10 | 2 |
| Compressive strength | at least 450 MPa | 3 |
| Thermal diffusivity | 0.655 mm²/s (0.63–0.68) | 4 |
| How it breaks | conchoidal | 1 |

**Missing:** fracture toughness (source page dead), specific heat, thermal conductivity.

**Sources**, all fetched 2026-10-01:

1. [Wikipedia: Obsidian](https://en.wikipedia.org/wiki/Obsidian), CC BY-SA 4.0
2. [Wagner and Heide 2005, arXiv](https://arxiv.org/pdf/cond-mat/0507559)
3. [Smith and others, EGU 2007 abstract](https://meetings.copernicus.org/www.cosis.net/abstracts/EGU2007/04479/EGU2007-J-04479-1.pdf)
4. [Romine and others 2012, abstract](https://corescholar.libraries.wright.edu/biology/288/)

<details>
<summary>Machine data, with the quotes</summary>

```yaml
id: obsidian
name: Obsidian
catalogue: ingredients
layer: 1
summary: >-
  Natural volcanic glass, mostly silica, formed when lava cools too fast to
  grow crystals. Like all glass it breaks with shell-like (conchoidal)
  fractures, giving the sharpest edges of any stone.
checks: [RCK-01]
properties:
  - property: density
    value: 2.4
    qualifier: about
    unit: g/cm3
    condition: specific gravity, which equals density in g/cm³
    source: wikipedia-obsidian
    quote: "Specific gravity c. 2.4[3]"
    note: Wikipedia cites Ericson and others 1975 (J. Non-Crystalline Solids), which is paywalled.
  - property: hardness
    value: 5.5
    low: 5
    high: 6
    basis: midpoint
    unit: Mohs
    condition: Mohs scale
    source: wikipedia-obsidian
    quote: "Fracture Conchoidal Mohs scale hardness 5-6[2]"
    note: Wikipedia cites a book (Moorey 1999) that is not online.
  - property: youngs_modulus
    value: 70
    plusminus: 10
    unit: GPa
    condition: natural volcanic glasses at room temperature, flexure pendulum at about 0.63 Hz
    source: wagner-heide-2005
    quote: "The Young's modulus at room temperature MRT = (70 ± 10)GPa is nearly constant."
  - property: compressive_strength
    value: 450
    qualifier: at least
    unit: MPa
    condition: crystal-free obsidian from Krafla, Iceland
    source: smith-egu-2007
    quote: >-
      the compressive strength of crystalline andesite is considerably lower
      than that of the obsidian (100 MPa vs >450 MPa)
  - property: thermal_diffusivity
    value: 0.655
    low: 0.63
    high: 0.68
    basis: midpoint
    unit: mm2/s
    condition: obsidian glass from Mono Craters, California, at room temperature
    source: romine-2012
    quote: "At room temperature, D glass varies from 0.63 to 0.68 mm2 s-1"
  - property: fracture_habit
    value: conchoidal
    unit: "-"
    source: wikipedia-obsidian
    quote: "Like all glass and some other naturally occurring rocks, obsidian breaks with a characteristic conchoidal fracture."
gaps:
  - property: fracture_toughness
    note: >-
      Search found 1.3 to 1.7 MN/m^1.5 in a thesis abstract on shareok.org,
      but the page returns 404 to both fetch tools, so it can't be checked.
  - property: specific_heat
    note: Not found in 2 searches; the likely papers are paywalled.
  - property: thermal_conductivity
    note: >-
      Only given as curves in a figure (USGS OFR 88-441) or for melts. Could
      be computed from diffusivity, density and specific heat once those exist.
sources:
  wikipedia-obsidian:
    title: "Obsidian (Wikipedia)"
    url: https://en.wikipedia.org/wiki/Obsidian
    fetched: 2026-10-01
    kind: encyclopedia
    publisher: Wikipedia
    licence: CC BY-SA 4.0
  wagner-heide-2005:
    title: "Mechanical spectroscopy on volcanic glasses (arXiv cond-mat/0507559)"
    url: https://arxiv.org/pdf/cond-mat/0507559
    fetched: 2026-10-01
    kind: paper
    publisher: arXiv
    licence: arXiv non-exclusive licence; cite single values
  smith-egu-2007:
    title: "The high temperature fracture mechanics of silicic magma: a comparison of crystalline andesite and rhyolitic obsidian (EGU 2007 abstract)"
    url: https://meetings.copernicus.org/www.cosis.net/abstracts/EGU2007/04479/EGU2007-J-04479-1.pdf
    fetched: 2026-10-01
    kind: paper
    publisher: European Geosciences Union
    licence: copyright EGU; cite single values
  romine-2012:
    title: "Thermal diffusivity of rhyolitic glasses and melts: effects of temperature, crystals and dissolved water (abstract)"
    url: https://corescholar.libraries.wright.edu/biology/288/
    fetched: 2026-10-01
    kind: paper
    publisher: Bulletin of Volcanology (Springer), 2012; abstract on Wright State repository
    licence: abstract only; cite single values
```

</details>
