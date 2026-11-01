# Symulacja zderzeń pp przy √s = 13.6 TeV

Symulacja i analiza rozkładów cząstek w zderzeniach proton–proton przy energii środka masy √s = 13.6 TeV. Pythia 8 generuje zdarzenia minimum bias (`SoftQCD:all`). Program w C++17 i ROOT 6 wybiera cząstki stanu końcowego, liczy krotność oraz rozkłady pT, η i φ i zapisuje histogramy do `analysis.root`. Makro `draw_plots.C` eksportuje wykresy do PNG i PDF.

## Wymagania

- `g++` i standard C++17
- Pythia 8.318 
- ROOT 6 

`Makefile` nie przypina numeru poprawki ROOT. Wybiera Pythię przez `PYTHIA8`, a ROOT przez `root-config` z `PATH`.

## Struktura projektu

| Ścieżka | Przeznaczenie |
| --- | --- |
| `Makefile` | Buduje `generate` i `analyze`: `g++`, flagi z `pythia8-config` i `root-config`, `-std=c++17` |
| `generate.cc` | Generator Pythia. Zapisuje drzewo zdarzeń do `event.root` |
| `analyze.cc` | Czyta `event.root`, wypełnia histogramy i zapisuje `analysis.root` |
| `draw_plots.C` | Czyta `analysis.root` i zapisuje wykresy w `plots/` |
| `event.root` | Wynik generatora: drzewo `events` (`pp_collisions_data`) |
| `analysis.root` | Wynik analizy: 32 histogramy `TH1D` |
| `plots/` | Wykresy PNG i PDF |
| `.vscode/c_cpp_properties.json` | Ścieżki nagłówków Pythia i ROOT oraz standard C++17 |
| `proton-proton_prezentacja_rozkładów.pptx` | Prezentacja rozkładów |

Po kompilacji w katalogu projektu powstają pliki wykonywalne `generate` i `analyze`.

## Proces uruchomienia

Polecenia należy wykonać w katalogu głównym repozytorium. Programy używają ścieżek względnych `event.root`, `analysis.root` i `plots/`.

Jeżeli `root-config` nie jest w `PATH`, wczytaj środowisko ROOT używane w tym projekcie:

```bash
source "$HOME/cern/root/bin/thisroot.sh"
```

Kompilacja obu programów:

```bash
make generate
make analyze
```

Inna lokalizacja Pythii:

```bash
make PYTHIA8=/ścieżka/do/pythia generate
make PYTHIA8=/ścieżka/do/pythia analyze
```

Generator:

```bash
./generate
```
`generate` tworzy drzewo i zapisuje je do event.root
Analiza:

```bash
./analyze
```

`analyze` czyta `event.root` i tworzy `analysis.root`.

Wykresy:

```bash
root -l -b -q draw_plots.C
```

Makro otwiera `analysis.root`, tworzy katalog `plots/` i dla każdego histogramu zapisuje `plots/<nazwa>.png` oraz `plots/<nazwa>.pdf`. Dodatkowo zapisuje wykresy zbiorcze `plots/hN_combined`, `plots/hPt_combined`, `plots/hEta_combined` i `plots/hPhi_combined`, każdy w PNG i PDF.

## Uwagi

Ziarno generatora jest stałe: `Random:setSeed = on` i `Random:seed = 100`.

Pythia zwraca kąt azymutalny w przedziale (−π, π]. Przed zapisem do drzewa wartość ujemna jest zamieniana na dodatnią (`std::fabs`). Tak przekształcone φ trafia do histogramu `hPhi_*` o zakresie 0–π i 64 binach.
