# Symulacja zderzeń pp przy √s = 13.6 TeV

Symulacja i analiza rozkładów cząstek w zderzeniach proton–proton przy energii środka masy √s = 13.6 TeV. Pythia 8 generuje zdarzenia minimum bias (`SoftQCD:all`). Program w C++17 i ROOT 6 wybiera cząstki stanu końcowego, liczy krotność oraz rozkłady pT, η i φ i zapisuje histogramy do `analysis.root`. Makro `draw_plots.C` eksportuje wykresy do PNG i PDF.

## Wymagania

- `g++` i standard C++17 (`-std=c++17` w `Makefile`; w `.vscode/c_cpp_properties.json` ustawione `cppStandard`: `c++17`)
- Pythia 8.318. `Makefile` woła `$(PYTHIA8)/bin/pythia8-config`. Domyślnie `PYTHIA8` to `$HOME/cern/pythia8318`
- ROOT 6 przez `root-config` oraz polecenie `root`. W tym środowisku `root-config --version` zwraca 6.40.04

`Makefile` nie przypina numeru poprawki ROOT. Wybiera Pythię przez `PYTHIA8`, a ROOT przez `root-config` z `PATH`.

## Struktura projektu

| Ścieżka | Przeznaczenie |
| --- | --- |
| `Makefile` | Buduje `generate2` i `analyze`: `g++`, flagi z `pythia8-config` i `root-config`, `-std=c++17` |
| `generate2.cc` | Generator Pythia. Zapisuje drzewo zdarzeń do `event.root` |
| `analyze.cc` | Czyta `event.root`, wypełnia histogramy i zapisuje `analysis.root` |
| `draw_plots.C` | Czyta `analysis.root` i zapisuje wykresy w `plots/` |
| `event.root` | Wynik generatora: drzewo `events` (`pp_collisions_data`) |
| `analysis.root` | Wynik analizy: 32 histogramy `TH1D` |
| `plots/` | Wykresy PNG i PDF |
| `.vscode/c_cpp_properties.json` | Ścieżki nagłówków Pythia i ROOT oraz standard C++17 |
| `proton-proton_prezentacja_rozkładów.pptx` | Prezentacja rozkładów |

Po kompilacji w katalogu projektu powstają pliki wykonywalne `generate2` i `analyze`.

## Proces uruchomienia

Polecenia należy wykonać w katalogu głównym repozytorium. Programy używają ścieżek względnych `event.root`, `analysis.root` i `plots/`.

Jeżeli `root-config` nie jest w `PATH`, wczytaj środowisko ROOT używane w tym projekcie:

```bash
source "$HOME/cern/root/bin/thisroot.sh"
```

Kompilacja obu programów:

```bash
make generate2
make analyze
```

Inna lokalizacja Pythii:

```bash
make PYTHIA8=/ścieżka/do/pythia generate2
make PYTHIA8=/ścieżka/do/pythia analyze
```

Generator:

```bash
./generate2
```

`generate2` ustawia zderzenia proton–proton (`Beams:idA = 2212`, `Beams:idB = 2212`), energię `Beams:eCM = 13600` oraz `SoftQCD:all = on`. Prosi Pythię o 100000 zdarzeń. Dla każdego udanego `pythia.next()` dopisuje jeden wpis do drzewa `events` w `event.root`. W obecnym pliku jest 100000 wpisów. Do drzewa wchodzą wyłącznie cząstki `isFinal()`, od indeksu 1 rekordu zdarzenia. Gałęzie: `pt`, `eta`, `phi`, `isCharged`, `pdgID`.

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

### Kategorie i PDG

| Kategoria | Wybór |
| --- | --- |
| `all` | wszystkie cząstki stanu końcowego |
| `ch` | cząstki z `isCharged()` (ładunek różny od zera, dowolny gatunek) |
| `pip`, `pim` | PDG `211`, `-211` |
| `Kp`, `Km` | PDG `321`, `-321` |
| `p`, `pbar` | PDG `2212`, `-2212` |

Krotność (`hN_*`) jest wypełniana raz na zdarzenie. Rozkłady pT, η i φ są wypełniane raz na cząstkę. Wskazane hadrony wchodzą też do `all` oraz, ponieważ są naładowane, do `ch`.

### Histogramy

| Histogram | Zakres | Liczba binów |
| --- | --- | --- |
| `hN_*` | 0–400 | 400 |
| `hPt_*` | 0–10 GeV | 100 |
| `hEta_*` | −5–5 | 100 |
| `hPhi_*` | 0–π | 64 |

Osiem sufiksów: `all`, `ch`, `pip`, `pim`, `Kp`, `Km`, `p`, `pbar`. Przykłady: `hN_all`, `hN_ch`, `hPt_pip`, `hEta_Km`, `hPhi_pbar`.

Na wykresach oś Y ma podpis `Entries`. Oś X: `Multiplicity N`, `p_{T} [GeV]`, `η` albo `φ [rad]`. Krotność i pT są rysowane ze skalą logarytmiczną na osi Y. Wykresy zbiorcze mają legendę kategorii.

## Uwagi

Ziarno generatora jest stałe: `Random:setSeed = on` i `Random:seed = 100`.

Pythia zwraca kąt azymutalny w przedziale (−π, π]. Przed zapisem do drzewa wartość ujemna jest zamieniana na dodatnią (`std::fabs`). Tak przekształcone φ trafia do histogramu `hPhi_*` o zakresie 0–π i 64 binach.
