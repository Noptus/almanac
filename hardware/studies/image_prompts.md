# Almanac Stone — image-generator prompt pack (shape variants)

Paste any prompt into an image model (Midjourney, DALL·E, SDXL, Firefly…) to
get a hero render of that variant. They share a **common style block** so the
outputs are comparable — only the *shape sentence* changes. PNG silhouette
studies of the raw geometry are alongside this file (`asym.png`, `house.png`,
`lectern.png`, `taper.png`, `trunc.png`, `tetra.png`, `iso.png`).

My proposals (per your "you propose"):
- **Crystal placement:** default to **crystal as one triangular end** — it
  stops the crystal competing with the wide screen for face space and looks
  magical in profile. Two variants below place it differently for comparison.
- **Edges:** default to **softly rounded edges + a narrow truncated top ridge**
  for engraving — the combination that reads most premium and least "doorstop."

---

## COMMON STYLE BLOCK (prepend or append to every shape prompt)

> Product photography of a small premium desk object, matte black cast-stone
> or dark walnut body with fine brushed-brass constellation inlay, a warm
> softly-glowing crystal, and a wide e-ink screen showing the date "20 August"
> and one short serif line of text. Sitting on a dark wood desk, moody dawn
> lighting, shallow depth of field, elegant, understated, mystical but tasteful,
> not gaudy. Studio catalog look, 3/4 angle, high detail, photorealistic.
> Aspect 3:2.

Keep this identical across variants so only the **shape** differs.

---

## 1 · Asymmetric wedge  *(recommended)*  — crystal as an end
`asym.png`

> …a triangular-prism desk piece lying on its base, with a STEEP short back
> edge and a LONG gentle front slope that carries the wide e-ink screen at a
> comfortable reading tilt; one triangular END of the prism is a faceted
> glowing crystal (amethyst) lit from within; edges softly rounded; the body
> matte black with thin brass constellation lines. …

## 2 · Right-triangle lectern — crystal on a front shelf
`lectern.png`

> …a right-triangle prism like a small folded lectern: a VERTICAL back face and
> a single SLOPED front face holding the wide e-ink screen facing up; a slim
> ledge at the base of the screen cradles a small standing crystal point; soft
> rounded edges; matte black body, brass star inlay on the vertical back. …

## 3 · Pentagonal "house" prism — crystal on the shelf
`house.png`

> …a five-sided prism: a short vertical lip at the front bottom forms a shelf,
> above it a sloped face holds the wide e-ink screen, rising to a ridge and a
> vertical back; a crystal rests on the front shelf below the screen; gently
> rounded edges; matte charcoal body with brass constellation inlay. …

## 4 · Tapered prism (head & tail) — crystal at the wide end
`taper.png`

> …a triangular prism that gently TAPERS along its length, wider and taller at
> one end (an obelisk laid on its side); the wide end is capped by a glowing
> crystal; the long sloped face carries the wide e-ink screen; rounded edges;
> matte black with fine brass celestial lines; calm, monumental. …

## 5 · Truncated-apex wedge — engraved top ridge
`trunc.png`

> …an asymmetric triangular prism whose top ridge is sliced into a narrow FLAT
> facet engraved with a small brass constellation and a name; the long front
> slope holds the wide e-ink screen; a lit crystal set into the left triangular
> end; crisp edges softened only slightly; matte black cast body. …

## 6 · Tetrahedron monument — crystal apex
`tetra.png`

> …a low triangular pyramid (tetrahedron) resting on a triangular base, one
> face angled up to hold a wide e-ink screen, the top apex a small glowing
> crystal; matte black stone-like body with brushed-brass edge lines;
> monumental, mystical, minimal; like a modern standing stone. …

## 7 · Symmetric isosceles (current STL, for reference)
`iso.png`

> …a symmetric triangular-prism (Toblerone) desk piece, the front face holding
> a wide e-ink screen, a lit crystal recessed in the front-left; matte black
> body, brass constellation inlay; softly rounded edges. …

---

## How to use these to decide
1. Generate #1, #2, #3 first (the realistic front-runners). Same style block.
2. Judge on: does the crystal read magical without crowding the screen? does it
   look *crafted* (wood/stone) rather than 3D-printed? does it sit like it
   belongs on a nightstand?
3. Pick one; I'll turn that exact variant into fabrication files (laser-cut
   panels / wood cut-sheet / resin STL — your call on material).

## Reference to feed the model (for consistency)
Attach `vision.png` (repo root) as a style/reference image where the tool
supports it — it locks the matte-black + brass + crystal + wide-screen look so
variants stay in the same family.
