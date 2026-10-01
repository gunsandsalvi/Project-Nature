# Granite

Ingredient · matter layer 1 · checks `RCK-01`

A coarse-grained rock of interlocking quartz, feldspar and mica crystals, formed from slowly cooled magma. Strong and heavy, but it breaks along its grains instead of in smooth shells, so it can't be chipped into blades.

| Property | Value | Src |
|---|---|---|
| Density | 2.70 g/cm³ (2.65–2.75) | 1 |
| Fracture toughness | 1.48 MPa·m½ | 2 |
| Young's modulus | 52 GPa | 3 |
| Specific heat | 0.79 kJ/(kg·K) | 4 |
| Thermal conductivity | 2.85 W/(m·K) (1.7–4.0) | 5 |
| Compressive strength | at least 200 MPa | 1 |
| How it breaks | not conchoidal | 6 |

**Missing:** hardness (only its minerals have one).

**Sources**, all fetched 2026-10-01:

1. [Wikipedia: Granite](https://en.wikipedia.org/wiki/Granite), CC BY-SA 4.0
2. [Nasseri and others 2009, accepted version](https://discovery.ucl.ac.uk/id/eprint/10098172/1/Nasseri%20PAGeoph%202009.pdf)
3. [Engineering ToolBox: Young's modulus](https://www.engineeringtoolbox.com/young-modulus-d_417.html)
4. [Engineering ToolBox: specific heats](https://www.engineeringtoolbox.com/specific-heat-solids-d_154.html)
5. [Engineering ToolBox: thermal conductivity](https://www.engineeringtoolbox.com/thermal-conductivity-d_429.html)
6. [Alutiiq Museum: Chipped One](https://alutiiqmuseum.org/collection/Detail/word/635)

<details>
<summary>Machine data, with the quotes</summary>

```yaml
id: granite
name: Granite
catalogue: ingredients
layer: 1
summary: >-
  A coarse-grained rock of interlocking quartz, feldspar and mica crystals,
  formed from slowly cooled magma. Strong and heavy, but it breaks along its
  grains instead of in smooth shells, so it can't be chipped into blades.
checks: [RCK-01]
properties:
  - property: density
    value: 2.70
    low: 2.65
    high: 2.75
    basis: midpoint
    unit: g/cm3
    condition: average for granite
    source: wikipedia-granite
    quote: "The average density of granite is between 2.65 and 2.75 g/cm3"
  - property: fracture_toughness
    value: 1.48
    unit: MPa*m^0.5
    condition: Westerly granite at room temperature, chevron-notched Brazilian disc
    source: nasseri-2009
    quote: "peak fracture toughness of 1.48 MPa.m0.5, measured in the RT sample"
  - property: youngs_modulus
    value: 52
    unit: GPa
    condition: generic granite; aggregator gives no sample or method
    source: etb-modulus
    quote: >-
      Tensile Modulus (Young's Modulus, Modulus of Elasticity) - E - (GPa) ...
      Granite 52
  - property: specific_heat
    value: 0.79
    unit: kJ/(kg*K)
    condition: generic granite; aggregator gives no sample or temperature
    source: etb-specific-heat
    quote: "Specific Heat - cp- (Btu/(lbm oF)) (kcal/(kg oC)) (kJ/kgK) ... Granite 0.19 0.79"
  - property: thermal_conductivity
    value: 2.85
    low: 1.7
    high: 4.0
    basis: midpoint
    unit: W/(m*K)
    condition: generic granite, about 25 °C
    source: etb-conductivity
    quote: "Thermal Conductivity - k - W/(m K) Temperature 25 oC ... Granite 1.7 - 4.0"
  - property: compressive_strength
    value: 200
    qualifier: at least
    unit: MPa
    condition: usual granite
    source: wikipedia-granite
    quote: "its compressive strength usually lies above 200 MPa (29,000 psi)"
  - property: fracture_habit
    value: not conchoidal
    unit: "-"
    condition: inferred from the fact that it can't be knapped
    source: alutiiq-museum
    quote: >-
      Obsidian, basalt, and chert can be chipped into tools, but materials
      like granite and slate don't work for this purpose.
    match: [granite, "don't work"]
gaps:
  - property: hardness
    note: >-
      No rock-level value; hardness belongs to its minerals (quartz 7 on the
      Mohs scale, feldspar about 6). Needs a stated rule for mixtures (MAT-03).
sources:
  wikipedia-granite:
    title: "Granite (Wikipedia)"
    url: https://en.wikipedia.org/wiki/Granite
    fetched: 2026-10-01
    kind: encyclopedia
    publisher: Wikipedia
    licence: CC BY-SA 4.0
  nasseri-2009:
    title: "Common evolution of mechanical and transport properties in thermally cracked Westerly granite at elevated hydrostatic pressure (Pure and Applied Geophysics, 2009; accepted version)"
    url: https://discovery.ucl.ac.uk/id/eprint/10098172/1/Nasseri%20PAGeoph%202009.pdf
    doi: 10.1007/s00024-009-0476-3
    fetched: 2026-10-01
    kind: paper
    publisher: Springer; accepted version on UCL Discovery
    licence: author manuscript; cite single values
  etb-modulus:
    title: "Young's Modulus, Tensile Strength and Yield Strength Values for some Materials (The Engineering ToolBox)"
    url: https://www.engineeringtoolbox.com/young-modulus-d_417.html
    fetched: 2026-10-01
    kind: aggregator
    publisher: The Engineering ToolBox
    licence: copyright; cite single values
  etb-specific-heat:
    title: "Solids - Specific Heats (The Engineering ToolBox)"
    url: https://www.engineeringtoolbox.com/specific-heat-solids-d_154.html
    fetched: 2026-10-01
    kind: aggregator
    publisher: The Engineering ToolBox
    licence: copyright; cite single values
  etb-conductivity:
    title: "Thermal Conductivity of Selected Materials and Gases (The Engineering ToolBox)"
    url: https://www.engineeringtoolbox.com/thermal-conductivity-d_429.html
    fetched: 2026-10-01
    kind: aggregator
    publisher: The Engineering ToolBox
    licence: copyright; cite single values
  alutiiq-museum:
    title: "Chipped One - Ilaiyarngasqaq (Alutiiq Museum word of the week)"
    url: https://alutiiqmuseum.org/collection/Detail/word/635
    fetched: 2026-10-01
    kind: website
    publisher: Alutiiq Museum
    licence: copyright; short quotation
```

</details>
