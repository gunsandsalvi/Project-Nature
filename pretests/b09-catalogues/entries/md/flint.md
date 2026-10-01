# Flint

Ingredient · matter layer 1 · checks `RCK-01`, `RCK-10`

A dark, very fine-grained form of quartz (a kind of chert) found as nodules in chalk. It breaks with smooth, shell-like (conchoidal) fractures, which makes it the classic stone for knapped tools.

| Property | Value | Src |
|---|---|---|
| Density | 2.60 g/cm³ | 1 |
| Hardness (Mohs) | 7 Mohs | 2 |
| Fracture toughness | 1.77 MPa·m½ (1.71–1.85) | 3 |
| Young's modulus | 74.82 GPa ± 0.9 | 3 |
| Thermal conductivity | 8.1 mcal/(cm·s·°C) | 1 |
| How it breaks | conchoidal | 4 |

**Missing:** specific heat (no measurement for flint found).

**Sources**, all fetched 2026-10-01:

1. [USGS Open-File Report 88-690](https://pubs.usgs.gov/of/1988/0690/report.pdf), public domain
2. [Handbook of Mineralogy: Quartz](https://www.handbookofmineralogy.org/pdfs/quartz.pdf)
3. [Nickel and Schmidt 2022, PLOS ONE](https://doi.org/10.1371/journal.pone.0278643), CC BY 4.0
4. [Wikipedia: Conchoidal fracture](https://en.wikipedia.org/wiki/Conchoidal_fracture), CC BY-SA 4.0

<details>
<summary>Machine data, with the quotes</summary>

```yaml
id: flint
name: Flint
catalogue: ingredients
layer: 1
summary: >-
  A dark, very fine-grained form of quartz (a kind of chert) found as nodules
  in chalk. It breaks with smooth, shell-like (conchoidal) fractures, which
  makes it the classic stone for knapped tools.
checks: [RCK-01, RCK-10]
properties:
  - property: density
    value: 2.60
    unit: g/cm3
    condition: one sample from Dover Cliff, England, room-dry
    source: usgs-ofr-88-690
    quote: >-
      511 Flint............... 8.1 8.1 A/R A/R 2.60 30 30 25 " 100 Wards.
      Dover Cliff, England.
    note: Scanned table; the column is read as measured density.
  - property: hardness
    value: 7
    unit: Mohs
    condition: Mohs scale; value for quartz, the mineral flint is made of
    source: hom-quartz
    quote: "Hardness = 7, variable by direction and form."
  - property: fracture_toughness
    value: 1.77
    low: 1.71
    high: 1.85
    unit: MPa*m^0.5
    condition: >-
      unheated flint; indentation fracture resistance (Niihara formula) with
      200, 100 and 50 N indents
    source: nickel-schmidt-2022
    quote: >-
      K1c (MPa* √m) VC-12-05 Flint unheated 200 74.82 ± 0.9 166.5 ± 21.7 189
      2.61 (0.8688) 382 ± 11.5 1.71 ± 0.08 VC-12-05 Flint unheated 100 74.82
      ±0.9 166.5 ± 21.7 189 2.61 (0.8688) 240 ± 8.1 1.77 ± 0.09 VC-12-05 Flint
      unheated 50 74.82 ±0.9 166.5 ± 21.7 189 2.61 (0.8688) 145 ± 5.4 1.85 ± 0.1
  - property: youngs_modulus
    value: 74.82
    plusminus: 0.9
    unit: GPa
    condition: unheated flint, sample VC-12-05
    source: nickel-schmidt-2022
    quote: "E (GPa) ... VC-12-05 Flint unheated 200 74.82 ± 0.9"
  - property: thermal_conductivity
    value: 8.1
    unit: mcal/(cm*s*degC)
    condition: one sample, room-dry, about 30 °C
    source: usgs-ofr-88-690
    quote: >-
      511 Flint............... 8.1 8.1 A/R A/R 2.60 30 30 25 " 100 Wards.
      Dover Cliff, England.
    note: About 3.4 W/(m·K). Unit read from the scanned table header.
  - property: fracture_habit
    value: conchoidal
    unit: "-"
    source: wikipedia-conchoidal
    quote: >-
      A conchoidal fracture is a break or fracture of a brittle material that
      does not follow any natural planes of separation. ... Materials that
      break in this way include quartz, chert, flint
    match: [conchoidal, flint]
gaps:
  - property: specific_heat
    note: >-
      No measurement for flint or chert found (4 searches, about 4 minutes).
      The main review (Waples and Waples 2004) is paywalled. NIST gives only
      an equation for quartz, which would need a computed value.
sources:
  usgs-ofr-88-690:
    title: "Thermal conductivity of some rock-forming minerals: a tabulation (Open-File Report 88-690)"
    url: https://pubs.usgs.gov/of/1988/0690/report.pdf
    fetched: 2026-10-01
    kind: government
    publisher: U.S. Geological Survey
    licence: US public domain
  hom-quartz:
    title: "Handbook of Mineralogy: Quartz"
    url: https://www.handbookofmineralogy.org/pdfs/quartz.pdf
    fetched: 2026-10-01
    kind: handbook
    publisher: Mineral Data Publishing, 2001 (Mineralogical Society of America)
    licence: copyright; cite single values
  nickel-schmidt-2022:
    title: "Knapping force as a function of stone heat treatment (PLOS ONE, 2022)"
    url: https://www.ebi.ac.uk/europepmc/webservices/rest/PMC9718382/fullTextXML
    doi: 10.1371/journal.pone.0278643
    fetched: 2026-10-01
    kind: paper
    publisher: PLOS
    licence: CC BY 4.0
  wikipedia-conchoidal:
    title: "Conchoidal fracture (Wikipedia)"
    url: https://en.wikipedia.org/wiki/Conchoidal_fracture
    fetched: 2026-10-01
    kind: encyclopedia
    publisher: Wikipedia
    licence: CC BY-SA 4.0
```

</details>
