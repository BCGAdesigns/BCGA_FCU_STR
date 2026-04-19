# FCU Config Helper — V2 source

React + TypeScript source of the brand-agnostic FCU tuning wizard.
Live at **https://bcgaairsoft.com/fcu/wizard**.

Compatible with BCGA S8PA / D8PA, PolarStar F2 / Fusion, Wolverine INFERNO,
GATE PULSAR D and other HPA FCUs — the timings are named with BCGA nomenclature
(DN / DR / DP / DB) and the wizard itself prints the cross-brand mapping at the
end.

## What the wizard does

14 steps, data-driven, no account required:

1. Engine — S8PA (single-solenoid) or D8PA (dual-solenoid)
2. First-boot defaults vs. existing config
3. BB mass (grams)
4–6. Chrono sweep at 80 / 100 / 120 psi (3 shots each)
7. Hopup tight @ 120 psi (3 shots)
8. Target pressure + FPS or Joule
9. DP iterative tuning with numeric delta suggestions
10. **DN deep-dive** (Dwell Nozzle — D8PA only)
11. **DR deep-dive** (Dwell Return)
12. **DP deep-dive** (confirmation, read-only from step 9)
13. **DB deep-dive** (Debounce — D8PA only)
14. Results: FPS × Joule matrix across 0.20 / 0.25 / 0.28 / 0.30 / 0.32 g
    plus custom mass; safety + cross-brand reference; PDF export;
    save-to-account (logged users) or JSON copy (anonymous).

## File

- `FCUWizard.tsx` — the React component, single file, ~1500 lines,
  PT/EN i18n inline, localStorage state (`bcga_fcu_helper_v2`),
  jsPDF lazy-imported.

## Dependencies

If you want to integrate the wizard into your own React app:

```
react ^19
react-router-dom ^7
jspdf ^2.5
tailwindcss ^3.4
```

Plus the bcgaairsoft project helpers used by the file:

- `@/components/Layout` — site chrome
- `@/components/SEO` — head meta
- `@/contexts/GeoContext` — `useGeo()` returns `{ country }` for pt/en
- `@/contexts/AuthContext` — `useAuth()` returns `{ user }`
- `@/lib/api` — `fetchFcuConfig`, `saveFcuConfig` (optional, for save feature)

If you don't need save-to-account, delete the `SaveModal` + `useAuth`
import + the `?load=` handling and the wizard runs fully standalone on
localStorage.

## Math reference

- FPS at target PSI: linear regression over the 3 measured PSI points
  (±10 psi extrapolation tolerance; beyond that the UI warns).
- FPS for other BB masses (energy conservation):
  `fps(m) = fps_ref × √(m_ref / m)`
- Joule:
  `J = 0.5 × (m / 1000) × (fps × 0.3048)²`

## License

MIT — same as the rest of the repo. Attribution appreciated if you fork
or port the wizard to another platform.
