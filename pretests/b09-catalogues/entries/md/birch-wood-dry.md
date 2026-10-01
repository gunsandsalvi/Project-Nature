# Birch wood, dry (yellow birch)

Ingredient · matter layer 1 · checks `RCK-02`, `RCK-12`

Seasoned wood of yellow birch, a hard, fairly heavy hardwood of northern forests, at about 12% moisture. Burns hot; its bark is the source of birch tar. Wood values below are for yellow birch unless the note says "wood in general".

| Property | Value | Src |
|---|---|---|
| Density | 0.62 g/cm³ | 2 |
| Hardness (Janka) | 5,600 N | 2 |
| Young's modulus | 13,900 MPa | 2 |
| Fracture toughness | 385 kPa·m½ (220–550) | 2 |
| Specific heat | 1.7 kJ/(kg·K) | 1 |
| Thermal conductivity | 0.18 W/(m·K) | 1 |
| Heat of combustion | about 20 MJ/kg | 3 |
| Ignition temperature | 350 °C (300–400) | 3 |

Fracture toughness, specific heat, heat of combustion and ignition temperature are for wood in general.

**Sources**, all fetched 2026-10-01, all from the USDA Wood Handbook (2010), public domain:

1. [Chapter 4: moisture relations and physical properties](https://research.fs.usda.gov/download/treesearch/37428.pdf)
2. [Chapter 5: mechanical properties](https://research.fs.usda.gov/download/treesearch/37427.pdf)
3. [Chapter 18: fire safety](https://research.fs.usda.gov/download/treesearch/37415.pdf)

<details>
<summary>Machine data, with the quotes</summary>

```yaml
id: birch-wood-dry
name: Birch wood, dry (yellow birch)
catalogue: ingredients
layer: 1
summary: >-
  Seasoned wood of yellow birch, a hard, fairly heavy hardwood of northern
  forests, at about 12% moisture. Burns hot; its bark is the source of birch
  tar. Wood values below are for yellow birch unless the note says "wood in
  general".
checks: [RCK-02, RCK-12]
properties:
  - property: density
    value: 0.62
    unit: g/cm3
    condition: specific gravity basis, oven-dry mass over volume at 12% moisture
    source: wh-ch5
    quote: >-
      names content gravityb (kPa) (MPa) ... Yellow Green 0.55 57,000 10,300
      111 1,220 23,300 3,000 7,700 3,000 3,600 12% 0.62 114,000 13,900 143
      1,400 56,300 6,700 13,000 6,300 5,600
  - property: hardness
    value: 5600
    unit: N
    condition: side hardness (Janka), 12% moisture
    source: wh-ch5
    quote: >-
      names content gravityb (kPa) (MPa) ... Yellow Green 0.55 57,000 10,300
      111 1,220 23,300 3,000 7,700 3,000 3,600 12% 0.62 114,000 13,900 143
      1,400 56,300 6,700 13,000 6,300 5,600
  - property: youngs_modulus
    value: 13900
    unit: MPa
    condition: modulus of elasticity in static bending, along the grain, 12% moisture
    source: wh-ch5
    quote: >-
      names content gravityb (kPa) (MPa) ... Yellow Green 0.55 57,000 10,300
      111 1,220 23,300 3,000 7,700 3,000 3,600 12% 0.62 114,000 13,900 143
      1,400 56,300 6,700 13,000 6,300 5,600
  - property: fracture_toughness
    value: 385
    low: 220
    high: 550
    basis: midpoint
    unit: kPa*m^0.5
    condition: wood in general, mode I (crack running along the grain)
    source: wh-ch5
    quote: >-
      Values for mode I fracture toughness range from 220 to 550 kPa m1/2
      (200 to 500 lbf in-2 in1/2)
  - property: specific_heat
    value: 1.7
    unit: kJ/(kg*K)
    condition: >-
      wood in general at 27 °C and 12% moisture (1.3 when oven-dry); the
      handbook says it barely depends on species
    source: wh-ch4
    quote: >-
      Heat capacity (kJ kg-1 K-1 (Btu lb-1 °F-1)) ... 300 27 (80) 1.3 (0.30)
      1.4 (0.34) 1.7 (0.40) 1.9 (0.45)
  - property: thermal_conductivity
    value: 0.18
    unit: W/(m*K)
    condition: across the grain, 12% moisture (0.15 when oven-dry)
    source: wh-ch4
    quote: >-
      Yellow 0.66 0.15 (1.0) 0.18 (1.2) 6.8 (0.98) 5.6 (0.81)
  - property: heat_of_combustion
    value: 20
    qualifier: about
    unit: MJ/kg
    condition: wood in general, oven-dry, gross (higher) heating value
    source: wh-ch18
    quote: >-
      The typical gross heat of combustion averaged around 20 MJ kg-1 for
      ovendried wood, depending on the lignin and extractive content of the
      wood.
  - property: ignition_temperature
    value: 350
    low: 300
    high: 400
    basis: midpoint
    unit: degC
    condition: wood in general, surface temperature at piloted ignition (with a flame nearby)
    source: wh-ch18
    quote: >-
      the surface temperature of wood materials has been measured in the
      range of 300 to 400 °C prior to piloted ignition
sources:
  wh-ch4:
    title: "Wood Handbook (FPL-GTR-190, 2010), chapter 4: Moisture relations and physical properties of wood"
    url: https://research.fs.usda.gov/download/treesearch/37428.pdf
    fetched: 2026-10-01
    kind: handbook
    publisher: USDA Forest Service, Forest Products Laboratory
    licence: US public domain
  wh-ch5:
    title: "Wood Handbook (FPL-GTR-190, 2010), chapter 5: Mechanical properties of wood"
    url: https://research.fs.usda.gov/download/treesearch/37427.pdf
    fetched: 2026-10-01
    kind: handbook
    publisher: USDA Forest Service, Forest Products Laboratory
    licence: US public domain
  wh-ch18:
    title: "Wood Handbook (FPL-GTR-190, 2010), chapter 18: Fire safety of wood construction"
    url: https://research.fs.usda.gov/download/treesearch/37415.pdf
    fetched: 2026-10-01
    kind: handbook
    publisher: USDA Forest Service, Forest Products Laboratory
    licence: US public domain
```

</details>
