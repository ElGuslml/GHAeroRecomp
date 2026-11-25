// This game's own texts (game.h): the first run's screens and the Instruments and gamepad page in the game's spirit
// (from GH Metallica's launcher, whose mockup they follow), the Game page's switches, where the game shows its DLC, and the
// guitar's functions on the Keyboard and Gamepad pages, in the launcher's languages (English, German, French, Spanish,
// Italian).

#include "game.h"

#include "settings.h"

namespace launcher::game {

const GameTexts& GameTextsFor(uint32_t language) {
  static const GameTexts kEnglish{
      .welcome_title = "Welcome backstage",
      .welcome_text = "Before you hit the stage, the game has to be installed. The launcher extracts its files from "
                      "your Xbox 360 ISO; you only have to do this once.",
      .toggles = {
          "Skip the intro videos", "The game starts at its title screen, without the logos and the intro.",
          "Game debug menu", "Neversoft's debug menu in the main menu and the pause menu.",
          "Bloom", "The glow around lights and bright surfaces.",
          "Depth of field", "The background blurred behind what is in focus.",
          "Whammy sound effect", "The whammy bar bends the pitch of your sustained notes.",
          "Lower the track on a miss", "Your part quieter after a miss, as on the console.",
          "Notes at the console's speed", "The notes reach the strike line as fast as at 100 %.",
      },
      .toggle_help = {
          "Skips the logos and the game's intro video at each start: the game opens at its title screen once it has loaded.",
          "Adds a Debug Menu item to the main menu and a Debug item to the pause menu: Neversoft's own menu (Unlock All, Play Song, Skip Into Song, Character Select, Play Credits, Settings with the bot and the song's time, ...). Its texts are in English. Unlock All changes the save as playing does; some items need Neversoft's development tools and do nothing. The development shortcuts the same switch had (unlocking or completing songs from the setlist) stay off.",
          "The halo of light around the stage's lights, the pyrotechnics and the brightest surfaces. Off, the image is sharper and flatter.",
          "Blurs the background in some camera shots and in the guitar selection, as a camera's lens does. Off, everything stays sharp.",
          "On, as on the console, moving the whammy bar on a sustained note bends your instrument's pitch. Off, the note keeps its pitch; the whammy bar still wobbles the note on the highway and still fills Star Power on Star Power notes.",
          "On, as on the console, a missed note or a strum without one lowers your instrument's track until you hit a note again; the volume under it says how much is left (0 % silences it). Off, your track never changes.",
          "With a perspective other than 100 %, the notes take the same time to cross the highway, so they reach the strike line slower below 100 % (about a fifth slower at 75 %) and faster above it. On, the time on the highway changes with the perspective so they reach it at the console's speed: below 100 % each note shows for less time (on Expert 1.18 s instead of 1.5 at 75 %; a longer highway gives it back), above 100 % for longer. The game's Hyperspeed works on top of it as usual. With two or more players their highways' own perspectives make the notes a little faster than on the console (4-7 % at 75 %). Off, the console's times.",
      },
      .choices = {
          "Highway scale", "The note highway's size when you play alone.",
          "Highway length", "Notes seen earlier, from further away.",
          "Highway perspective", "How tilted the highway looks: flatter or steeper.",
          "Highway style", "The look of the highway behind the notes.",
          "Highway opacity", "How much of the venue shows through the highway.",
          "Instrument volume on a miss", "How loud your part stays when you miss a note.",
      },
      .choice_help = {
          "Makes the note highway smaller or larger when you play alone, anchored at the strike line: the gems, the sidebars and the strikers with it; the rock meter and the Star Power meter stay where they are. At 80 % more of the stage shows around it. 100 % is the console's size; with two players the game's own sizes stay.",
          "Shows each note that much earlier, as in Clone Hero: the console's highway continues beyond its far end in its own perspective, so it gets a little taller and the extra notes come from further away, while every note still reaches the strike line at the same speed. On Expert a note normally shows 1.5 s before you play it; at 150 %, 2.25 s. The timing to hit the notes does not change. It applies to every player. With a flatter perspective the longer part takes more of the screen. 100 % is the console's highway.",
          "The highway's perspective, as if the camera looked from higher (below 100 %: the far end wider and its notes larger, more of the highway easy to read) or lower (above 100 %: the far end narrower and more compressed). The notes take the same time to cross the highway, so below 100 % they move slower near the strike line (about a fifth slower at 75 %) and above 100 % faster: the switch under it keeps the console's speed, or the game's Hyperspeed makes up for it. The length above continues the same perspective. 100 % is the console's.",
          "Dark is the community's black highway: the highway's picture turned black, so the gems, the sustains and the lines stand out against it.",
          "Below 100 % the venue shows through the note highway: the notes, the lines and the sidebars stay solid. 100 % is the console's opaque highway.",
          "When you miss a note or strum without one, the game mutes your instrument's track until you hit a note again. Here you choose how much of it stays: 0 % mutes it as on the console, 50 % halves it, 100 % keeps it as it was. The lead guitar and the bass or rhythm guitar; with two players on the same part (they share its track) it does not change.",
      },
      .choice_values = {{{}, {}, {}, {"Original", "Dark"}, {}, {}}},
      .done_title = "The stage is yours!",
      .done_text = "The game was installed. Go play now, or first review the video, graphics and instrument settings; "
                   "you can always change them in the launcher.",
      .dlc_where = "The game lists the songs in Quickplay: in the song list, with the blue button.",
      .extracting_title = "Tuning up the stage",
      .extracting_text = "The game is being extracted. It can take a few minutes; do not close the launcher.",
      .keyboard_tap_title = "Strum when a fret is pressed",
      .keyboard_tap_hint = "Each fret key strums as you press it; frets pressed within 20 ms of each other are one chord, one strum.",
  };
  static const GameTexts kGerman{
      .welcome_title = "Willkommen im Backstage",
      .welcome_text = "Bevor es auf die Bühne geht, muss das Spiel installiert werden. Der Launcher entpackt die Dateien "
                      "aus deinem Xbox-360-ISO; das musst du nur einmal tun.",
      .toggles = {
          "Intro-Videos überspringen", "Das Spiel startet am Titelbildschirm, ohne Logos und Intro.",
          "Debug-Menü des Spiels", "Das Debug-Menü von Neversoft im Hauptmenü und im Pausenmenü.",
          "Bloom", "Das Leuchten um Lichter und helle Flächen.",
          "Tiefenschärfe", "Der unscharfe Hintergrund hinter dem Fokus.",
          "Whammy-Klangeffekt", "Der Whammy-Hebel verbiegt die Tonhöhe gehaltener Noten.",
          "Spur bei Fehlern leiser", "Dein Part nach einem Fehler leiser, wie auf der Konsole.",
          "Noten im Tempo der Konsole", "Die Noten erreichen die Schlaglinie so schnell wie bei 100 %.",
      },
      .toggle_help = {
          "Überspringt bei jedem Start die Logos und das Intro-Video des Spiels: Das Spiel öffnet nach dem Laden am Titelbildschirm.",
          "Fügt dem Hauptmenü den Eintrag Debug Menu und dem Pausenmenü den Eintrag Debug hinzu: Neversofts eigenes Menü (Unlock All, Play Song, Skip Into Song, Character Select, Play Credits, Settings mit dem Bot und der Songzeit, ...). Seine Texte sind auf Englisch. Unlock All ändert den Spielstand wie das Spielen; manche Einträge brauchen Neversofts Entwicklungswerkzeuge und tun nichts. Die Entwicklerabkürzungen desselben Schalters (Songs in der Songliste freischalten oder abschließen) bleiben aus.",
          "Der Lichtschein um die Bühnenlichter, die Pyrotechnik und die hellsten Flächen. Aus wirkt das Bild schärfer und flacher.",
          "Macht den Hintergrund in manchen Kameraeinstellungen und in der Gitarrenauswahl unscharf, wie ein Kameraobjektiv. Aus bleibt alles scharf.",
          "An, wie auf der Konsole: Bewegst du den Whammy-Hebel auf einer gehaltenen Note, verbiegt er die Tonhöhe deines Instruments. Aus: Die Note behält ihre Tonhöhe; der Whammy-Hebel lässt die Note auf der Bahn weiter wackeln und füllt auf Star-Power-Noten weiter Star Power.",
          "An, wie auf der Konsole: Eine verpasste Note oder ein Anschlag ohne Note macht die Spur deines Instruments leiser, bis du wieder eine Note triffst; die Lautstärke darunter sagt, wie viel bleibt (0 % schaltet sie stumm). Aus: Deine Spur ändert sich nie.",
          "Mit einer Perspektive außer 100 % brauchen die Noten gleich lang über die Bahn, erreichen die Schlaglinie also unter 100 % langsamer (bei 75 % etwa ein Fünftel) und über 100 % schneller. Eingeschaltet ändert sich die Zeit auf der Bahn mit der Perspektive, sodass sie sie im Tempo der Konsole erreichen: Unter 100 % ist jede Note kürzer zu sehen (auf Experte bei 75 % 1,18 s statt 1,5; eine längere Bahn gleicht das aus), über 100 % länger. Hyperspeed im Spiel wirkt wie gewohnt zusätzlich. Mit zwei oder mehr Spielern sind die Noten durch die eigenen Perspektiven ihrer Bahnen etwas schneller als auf der Konsole (4-7 % bei 75 %). Aus: die Zeiten der Konsole.",
      },
      .choices = {
          "Notenbahn-Größe", "Die Größe der Notenbahn, wenn du allein spielst.",
          "Notenbahn-Länge", "Noten früher sehen, von weiter her.",
          "Notenbahn-Perspektive", "Wie geneigt die Bahn wirkt: flacher oder steiler.",
          "Notenbahn-Stil", "Das Aussehen der Bahn hinter den Noten.",
          "Deckkraft der Notenbahn", "Wie viel von der Location durch die Bahn scheint.",
          "Instrument-Lautstärke bei Fehlern", "Wie laut deine Spur bleibt, wenn du eine Note verpasst.",
      },
      .choice_help = {
          "Macht die Notenbahn kleiner oder größer, wenn du allein spielst, verankert an der Schlaglinie: Noten, Seitenleisten und Anschlagringe mit ihr; Rock-Meter und Star-Power-Anzeige bleiben, wo sie sind. Bei 80 % ist mehr von der Bühne zu sehen. 100 % ist die Größe der Konsole; mit zwei Spielern bleiben die Größen des Spiels.",
          "Zeigt jede Note entsprechend früher, wie in Clone Hero: Die Notenbahn der Konsole setzt sich über ihr fernes Ende hinaus in ihrer eigenen Perspektive fort, wird dadurch etwas höher, und die zusätzlichen Noten kommen von weiter her, während jede Note die Schlaglinie mit derselben Geschwindigkeit erreicht. Auf Experte erscheint eine Note normalerweise 1,5 s, bevor du sie spielst; bei 150 % 2,25 s. Das Timing zum Treffen der Noten ändert sich nicht. Gilt für alle Spieler. Mit einer flacheren Perspektive nimmt der längere Teil mehr vom Bildschirm ein. 100 % ist die Notenbahn der Konsole.",
          "Die Perspektive der Notenbahn, als blicke die Kamera von höher (unter 100 %: das ferne Ende breiter, seine Noten größer, mehr von der Bahn gut lesbar) oder von tiefer (über 100 %: das ferne Ende schmaler und gestauchter). Die Noten brauchen gleich lang über die Bahn, daher bewegen sie sich unter 100 % nahe der Schlaglinie langsamer (bei 75 % etwa ein Fünftel) und über 100 % schneller: Der Schalter darunter behält das Tempo der Konsole, oder Hyperspeed im Spiel gleicht das aus. Die Länge oben setzt dieselbe Perspektive fort. 100 % ist die der Konsole.",
          "Dunkel ist die schwarze Notenbahn der Community: Das Bild der Bahn wird schwarz, sodass Noten, gehaltene Noten und Linien hervorstechen.",
          "Unter 100 % scheint die Location durch die Notenbahn: Noten, Linien und Seitenleisten bleiben deckend. 100 % ist die deckende Bahn der Konsole.",
          "Verpasst du eine Note oder schlägst ohne eine an, schaltet das Spiel die Spur deines Instruments stumm, bis du wieder eine Note triffst. Hier wählst du, wie viel davon bleibt: 0 % schaltet sie stumm wie auf der Konsole, 50 % halbiert sie, 100 % lässt sie, wie sie war. Leadgitarre und Bass oder Rhythmusgitarre; spielen zwei Spieler denselben Part (sie teilen sich seine Spur), ändert sich nichts.",
      },
      .choice_values = {{{}, {}, {}, {"Original", "Dunkel"}, {}, {}}},
      .done_title = "Die Bühne gehört dir!",
      .done_text = "Das Spiel wurde installiert. Leg gleich los oder sieh dir zuerst die Video-, Grafik- und "
                   "Instrumenteinstellungen an; du kannst sie jederzeit im Launcher ändern.",
      .dlc_where = "Das Spiel zeigt die Songs im Schnellspiel: in der Songliste mit der blauen Taste.",
      .extracting_title = "Die Bühne wird gestimmt",
      .extracting_text = "Das Spiel wird entpackt. Das kann ein paar Minuten dauern; schließe den Launcher nicht.",
      .keyboard_tap_title = "Anschlag beim Drücken eines Bunds",
      .keyboard_tap_hint = "Jede Bundtaste schlägt an, wenn du sie drückst; Bünde, die du innerhalb von 20 ms drückst, sind ein Akkord, ein Anschlag.",
  };
  static const GameTexts kFrench{
      .welcome_title = "Bienvenue en coulisses",
      .welcome_text = "Avant de monter sur scène, il faut installer le jeu. Le launcher extrait ses fichiers de votre ISO "
                      "Xbox 360 ; vous n'aurez à le faire qu'une fois.",
      .toggles = {
          "Passer les vidéos d'intro", "Le jeu démarre sur son écran titre, sans les logos ni l'intro.",
          "Menu de débogage du jeu", "Le menu de débogage de Neversoft dans le menu principal et le menu pause.",
          "Bloom", "La lueur autour des lumières et des surfaces claires.",
          "Profondeur de champ", "L'arrière-plan flou derrière le sujet net.",
          "Effet sonore du vibrato", "La barre de vibrato fait varier la hauteur des notes tenues.",
          "Baisser la piste sur une erreur", "Votre partie moins forte après une erreur, comme sur la console.",
          "Notes à la vitesse de la console", "Les notes arrivent à la ligne de frappe aussi vite qu'à 100 %.",
      },
      .toggle_help = {
          "Passe à chaque démarrage les logos et la vidéo d'intro du jeu : le jeu s'ouvre sur son écran titre une fois chargé.",
          "Ajoute l'entrée Debug Menu au menu principal et l'entrée Debug au menu pause : le menu de Neversoft (Unlock All, Play Song, Skip Into Song, Character Select, Play Credits, Settings avec le bot et le temps de la chanson, ...). Ses textes sont en anglais. Unlock All modifie la sauvegarde comme le jeu ; certaines entrées demandent les outils de développement de Neversoft et ne font rien. Les raccourcis de développement du même interrupteur (débloquer ou terminer des chansons depuis la liste) restent désactivés.",
          "Le halo autour des lumières de la scène, de la pyrotechnie et des surfaces les plus claires. Désactivé, l'image est plus nette et plus plate.",
          "Floute l'arrière-plan dans certains plans de caméra et dans le choix de la guitare, comme un objectif. Désactivée, tout reste net.",
          "Activé, comme sur la console : bouger la barre de vibrato sur une note tenue fait varier la hauteur de votre instrument. Désactivé, la note garde sa hauteur ; la barre de vibrato fait toujours onduler la note sur la piste et remplit toujours le Star Power sur les notes Star Power.",
          "Activé, comme sur la console : une note ratée ou un grattement sans note baisse la piste de votre instrument jusqu'à la prochaine note réussie ; le volume dessous dit ce qu'il en reste (0 % la coupe). Désactivé, votre piste ne change jamais.",
          "Avec une perspective autre que 100 %, les notes mettent le même temps à parcourir la piste : elles arrivent donc à la ligne de frappe plus lentement sous 100 % (environ un cinquième de moins à 75 %) et plus vite au-dessus. Activé, le temps sur la piste change avec la perspective pour qu'elles y arrivent à la vitesse de la console : sous 100 % chaque note reste visible moins longtemps (en Expert 1,18 s au lieu de 1,5 à 75 % ; une piste plus longue le rend), au-dessus de 100 % plus longtemps. L'Hyperspeed du jeu s'ajoute comme d'habitude. À deux joueurs ou plus, les perspectives propres de leurs pistes rendent les notes un peu plus rapides que sur la console (4 à 7 % à 75 %). Désactivé, les temps de la console.",
      },
      .choices = {
          "Taille de la piste", "La taille de la piste de notes quand vous jouez seul.",
          "Longueur de la piste", "Les notes vues plus tôt, venant de plus loin.",
          "Perspective de la piste", "L'inclinaison de la piste : plus plate ou plus raide.",
          "Style de la piste", "L'aspect de la piste derrière les notes.",
          "Opacité de la piste", "Ce qu'on voit de la salle à travers la piste.",
          "Volume de l'instrument sur une erreur", "Le volume de votre partie quand vous ratez une note.",
      },
      .choice_help = {
          "Réduit ou agrandit la piste de notes quand vous jouez seul, ancrée à la ligne de frappe : les notes, les bords et les anneaux de frappe avec elle ; le rock-mètre et la jauge de Star Power restent à leur place. À 80 %, on voit davantage la scène autour. 100 % est la taille de la console ; à deux joueurs, les tailles du jeu restent.",
          "Montre chaque note d'autant plus tôt, comme dans Clone Hero : la piste de la console se prolonge au-delà de son bout lointain dans sa propre perspective, elle devient donc un peu plus haute et les notes en plus viennent de plus loin, tandis que chaque note arrive à la ligne de frappe à la même vitesse. En Expert, une note apparaît normalement 1,5 s avant que vous la jouiez ; à 150 %, 2,25 s. Le timing pour toucher les notes ne change pas. S'applique à tous les joueurs. Avec une perspective plus plate, la partie en plus prend davantage d'écran. 100 % est la piste de la console.",
          "La perspective de la piste, comme si la caméra regardait de plus haut (sous 100 % : le bout lointain plus large et ses notes plus grandes, plus de piste facile à lire) ou de plus bas (au-dessus de 100 % : le bout lointain plus étroit et plus tassé). Les notes mettent le même temps à parcourir la piste : sous 100 % elles avancent donc plus lentement près de la ligne de frappe (environ un cinquième de moins à 75 %), et plus vite au-dessus de 100 % ; l'interrupteur dessous garde la vitesse de la console, ou l'Hyperspeed du jeu le compense. La longueur ci-dessus prolonge la même perspective. 100 % est celle de la console.",
          "Sombre est la piste noire de la communauté : l'image de la piste devient noire, pour que les notes, les notes tenues et les lignes ressortent.",
          "Sous 100 %, la salle se voit à travers la piste de notes : les notes, les lignes et les bords restent opaques. 100 % est la piste opaque de la console.",
          "Quand vous ratez une note ou grattez sans note, le jeu coupe la piste de votre instrument jusqu'à la prochaine note réussie. Ici vous choisissez ce qu'il en reste : 0 % la coupe comme sur la console, 50 % la réduit de moitié, 100 % la laisse telle quelle. La guitare solo et la basse ou guitare rythmique ; avec deux joueurs sur la même partie (ils partagent sa piste), rien ne change.",
      },
      .choice_values = {{{}, {}, {}, {"Originale", "Sombre"}, {}, {}}},
      .done_title = "La scène est à vous !",
      .done_text = "Le jeu a été installé. Allez jouer tout de suite, ou vérifiez d'abord les réglages vidéo, "
                   "graphiques et de vos instruments ; vous pourrez toujours les changer dans le launcher.",
      .dlc_where = "Le jeu affiche les chansons en Partie rapide : dans la liste des chansons, avec le bouton bleu.",
      .extracting_title = "On accorde la scène",
      .extracting_text = "Le jeu est en cours d'extraction. Cela peut prendre quelques minutes ; ne fermez pas le "
                         "launcher.",
      .keyboard_tap_title = "Gratter en appuyant sur une frette",
      .keyboard_tap_hint = "Chaque touche de frette gratte quand vous l'appuyez ; les frettes appuyées à moins de 20 ms d'intervalle forment un accord, un seul grattage.",
  };
  static const GameTexts kSpanish{
      .welcome_title = "Bienvenido al backstage",
      .welcome_text = "Antes de subir al escenario hay que instalar el juego. El launcher extrae los archivos desde tu "
                      "ISO de Xbox 360; solo tendrás que hacerlo una vez.",
      .toggles = {
          "Saltar los videos de introducción", "El juego empieza en su pantalla de inicio, sin los logos ni la intro.",
          "Menú de depuración del juego", "El menú de depuración de Neversoft en el menú principal y en el de pausa.",
          "Bloom", "El resplandor alrededor de las luces y superficies claras.",
          "Profundidad de campo", "El fondo desenfocado detrás de lo enfocado.",
          "Efecto de sonido del whammy", "La palanca de whammy dobla el tono de tus notas largas.",
          "Bajar la pista al fallar", "Tu parte más baja después de fallar, como en la consola.",
          "Notas a la velocidad de la consola", "Las notas llegan a la línea de golpe tan rápido como al 100 %.",
      },
      .toggle_help = {
          "Salta en cada inicio los logos y el video de introducción del juego: el juego abre en su pantalla de inicio en cuanto carga.",
          "Añade la opción Debug Menu al menú principal y Debug al menú de pausa: el menú propio de Neversoft (Unlock All, Play Song, Skip Into Song, Character Select, Play Credits, Settings con el bot y el tiempo de la canción, ...). Sus textos están en inglés. Unlock All cambia la partida guardada como jugar; algunas opciones necesitan las herramientas de desarrollo de Neversoft y no hacen nada. Los atajos de desarrollo del mismo interruptor (desbloquear o completar canciones desde la lista) siguen desactivados.",
          "El halo de luz alrededor de las luces del escenario, la pirotecnia y las superficies más brillantes. Desactivado, la imagen se ve más nítida y plana.",
          "Desenfoca el fondo en algunas tomas de cámara y en la selección de guitarra, como el lente de una cámara. Desactivada, todo se ve nítido.",
          "Activado, como en la consola, mover la palanca de whammy en una nota larga dobla el tono de tu instrumento. Desactivado, la nota mantiene su tono; la palanca sigue haciendo ondular la nota en el highway y sigue llenando el Star Power en las notas de Star Power.",
          "Activado, como en la consola, una nota fallada o un rasgueo sin nota bajan la pista de tu instrumento hasta que aciertes otra; el volumen de abajo dice cuánto queda (0 % la silencia). Desactivado, tu pista nunca cambia.",
          "Con una perspectiva distinta de 100 % las notas tardan lo mismo en recorrer el highway, así que llegan a la línea de golpe más lento bajo 100 % (cerca de una quinta parte menos al 75 %) y más rápido sobre 100 %. Activado, el tiempo en el highway cambia con la perspectiva para que lleguen a la velocidad de la consola: bajo 100 % cada nota se ve menos tiempo (en Experto 1,18 s en vez de 1,5 al 75 %; un highway más largo lo devuelve), sobre 100 % más tiempo. El Hyperspeed del juego funciona encima como siempre. Con dos o más jugadores, las perspectivas propias de sus highways hacen las notas algo más rápidas que en la consola (4-7 % al 75 %). Desactivado, los tiempos de la consola.",
      },
      .choices = {
          "Escala del highway", "El tamaño del highway de notas cuando tocas solo.",
          "Largo del highway", "Las notas se ven antes, desde más lejos.",
          "Perspectiva del highway", "Qué tan inclinado se ve el highway: más plano o más empinado.",
          "Estilo del highway", "El aspecto del highway detrás de las notas.",
          "Opacidad del highway", "Cuánto del escenario se ve a través del highway.",
          "Volumen del instrumento al fallar", "Qué tan fuerte queda tu parte cuando fallas una nota.",
      },
      .choice_help = {
          "Hace el highway de notas más pequeño o más grande cuando tocas solo, anclado en la línea de golpe: las notas, los bordes y los aros de golpe con él; el medidor de rock y el de Star Power se quedan donde están. Al 80 % se ve más del escenario alrededor. 100 % es el tamaño de la consola; con dos jugadores se quedan los tamaños del juego.",
          "Muestra cada nota así de antes, como en Clone Hero: el highway de la consola continúa más allá de su extremo lejano en su propia perspectiva, así que se hace un poco más alto y las notas de más vienen desde más lejos, mientras cada nota llega a la línea de golpe a la misma velocidad. En Experto una nota aparece normalmente 1,5 s antes de tocarla; al 150 %, 2,25 s. El momento para acertar las notas no cambia. Vale para todos los jugadores. Con una perspectiva más plana, la parte de más ocupa más pantalla. 100 % es el highway de la consola.",
          "La perspectiva del highway, como si la cámara mirara desde más arriba (bajo 100 %: el extremo lejano más ancho y sus notas más grandes, se lee más del highway) o desde más abajo (sobre 100 %: el extremo lejano más angosto y más comprimido). Las notas tardan lo mismo en recorrer el highway, así que bajo 100 % se mueven más lento cerca de la línea de golpe (cerca de una quinta parte menos al 75 %) y sobre 100 % más rápido: el interruptor de abajo mantiene la velocidad de la consola, o el Hyperspeed del juego lo compensa. El largo de arriba continúa la misma perspectiva. 100 % es la de la consola.",
          "Oscuro es el highway negro de la comunidad: la imagen del highway se vuelve negra, así las notas, las notas largas y las líneas resaltan.",
          "Bajo 100 % el escenario se ve a través del highway de notas: las notas, las líneas y los bordes siguen opacos. 100 % es el highway opaco de la consola.",
          "Cuando fallas una nota o rasgueas sin nota, el juego silencia la pista de tu instrumento hasta que aciertes otra. Aquí eliges cuánto queda: 0 % la silencia como en la consola, 50 % la deja a la mitad, 100 % la deja como estaba. La guitarra principal y el bajo o la guitarra rítmica; con dos jugadores en la misma parte (comparten su pista) no cambia.",
      },
      .choice_values = {{{}, {}, {}, {"Original", "Oscuro"}, {}, {}}},
      .done_title = "¡El escenario es tuyo!",
      .done_text = "El juego se instaló correctamente. Puedes salir a tocar ya o revisar primero los ajustes de video, "
                   "gráficos y tus instrumentos; siempre podrás cambiarlos desde el launcher.",
      .dlc_where = "El juego muestra las canciones en Partida rápida: en el repertorio, el botón azul (Descarga de "
                   "contenido).",
      .extracting_title = "Afinando el escenario",
      .extracting_text = "Estamos extrayendo el juego. Puede tardar unos minutos; no cierres el launcher.",
      .keyboard_tap_title = "Activar rasgueo al presionar botón",
      .keyboard_tap_hint = "Cada tecla de traste rasguea al presionarla; los trastes que presiones con menos de 20 ms de diferencia cuentan como un acorde: un solo rasgueo.",
  };
  static const GameTexts kItalian{
      .welcome_title = "Benvenuto nel backstage",
      .welcome_text = "Prima di salire sul palco bisogna installare il gioco. Il launcher estrae i file dalla tua ISO "
                      "Xbox 360; dovrai farlo una sola volta.",
      .toggles = {
          "Salta i video introduttivi", "Il gioco parte dalla schermata del titolo, senza i loghi né l'intro.",
          "Menu di debug del gioco", "Il menu di debug di Neversoft nel menu principale e nel menu di pausa.",
          "Bloom", "Il bagliore attorno alle luci e alle superfici chiare.",
          "Profondità di campo", "Lo sfondo sfocato dietro ciò che è a fuoco.",
          "Effetto sonoro della leva whammy", "La leva whammy piega l'intonazione delle note lunghe.",
          "Abbassa la traccia sugli errori", "La tua parte più bassa dopo un errore, come sulla console.",
          "Note alla velocità della console", "Le note arrivano alla linea di battuta veloci come al 100 %.",
      },
      .toggle_help = {
          "Salta a ogni avvio i loghi e il video introduttivo del gioco: il gioco si apre sulla schermata del titolo appena caricato.",
          "Aggiunge la voce Debug Menu al menu principale e Debug al menu di pausa: il menu di Neversoft (Unlock All, Play Song, Skip Into Song, Character Select, Play Credits, Settings con il bot e il tempo della canzone, ...). I suoi testi sono in inglese. Unlock All modifica il salvataggio come il gioco; alcune voci richiedono gli strumenti di sviluppo di Neversoft e non fanno nulla. Le scorciatoie di sviluppo dello stesso interruttore (sbloccare o completare canzoni dalla scaletta) restano disattivate.",
          "L'alone di luce attorno alle luci del palco, alla pirotecnica e alle superfici più luminose. Disattivato, l'immagine è più nitida e piatta.",
          "Sfoca lo sfondo in alcune inquadrature e nella scelta della chitarra, come l'obiettivo di una telecamera. Disattivata, tutto resta nitido.",
          "Attivato, come sulla console, muovere la leva whammy su una nota lunga piega l'intonazione del tuo strumento. Disattivato, la nota mantiene la sua intonazione; la leva fa ancora ondeggiare la nota sulla pista e riempie ancora lo Star Power sulle note Star Power.",
          "Attivato, come sulla console, una nota sbagliata o una pennata senza nota abbassano la traccia del tuo strumento finché non colpisci di nuovo una nota; il volume sotto dice quanto ne resta (0 % la silenzia). Disattivato, la tua traccia non cambia mai.",
          "Con una prospettiva diversa dal 100 % le note impiegano lo stesso tempo a percorrere la pista, quindi arrivano alla linea di battuta più lente sotto il 100 % (circa un quinto in meno al 75 %) e più veloci sopra. Attivato, il tempo sulla pista cambia con la prospettiva perché vi arrivino alla velocità della console: sotto il 100 % ogni nota resta visibile meno a lungo (in Esperto 1,18 s invece di 1,5 al 75 %; una pista più lunga lo restituisce), sopra il 100 % più a lungo. L'Hyperspeed del gioco si aggiunge come sempre. Con due o più giocatori le prospettive proprie delle loro piste rendono le note un po' più veloci che sulla console (4-7 % al 75 %). Disattivato, i tempi della console.",
      },
      .choices = {
          "Dimensione della pista", "La dimensione della pista delle note quando suoni da solo.",
          "Lunghezza della pista", "Le note viste prima, da più lontano.",
          "Prospettiva della pista", "Quanto è inclinata la pista: più piatta o più ripida.",
          "Stile della pista", "L'aspetto della pista dietro le note.",
          "Opacità della pista", "Quanto del locale si vede attraverso la pista.",
          "Volume dello strumento sugli errori", "Quanto resta forte la tua parte quando sbagli una nota.",
      },
      .choice_help = {
          "Rende la pista delle note più piccola o più grande quando suoni da solo, ancorata alla linea di battuta: le note, i bordi e gli anelli di battuta con essa; il rock meter e l'indicatore di Star Power restano dove sono. All'80 % si vede più palco intorno. 100 % è la dimensione della console; con due giocatori restano le dimensioni del gioco.",
          "Mostra ogni nota altrettanto prima, come in Clone Hero: la pista della console prosegue oltre la sua estremità lontana nella sua stessa prospettiva, quindi diventa un po' più alta e le note in più arrivano da più lontano, mentre ogni nota arriva alla linea di battuta alla stessa velocità. In Esperto una nota appare normalmente 1,5 s prima di suonarla; al 150 %, 2,25 s. Il tempo per colpire le note non cambia. Vale per tutti i giocatori. Con una prospettiva più piatta la parte in più occupa più schermo. 100 % è la pista della console.",
          "La prospettiva della pista, come se la telecamera guardasse da più in alto (sotto il 100 %: l'estremità lontana più larga e le sue note più grandi, si legge più pista) o da più in basso (sopra il 100 %: l'estremità lontana più stretta e più compressa). Le note impiegano lo stesso tempo a percorrere la pista, quindi sotto il 100 % si muovono più lente vicino alla linea di battuta (circa un quinto in meno al 75 %) e sopra il 100 % più veloci: l'interruttore sotto mantiene la velocità della console, oppure l'Hyperspeed del gioco lo compensa. La lunghezza sopra prosegue la stessa prospettiva. 100 % è quella della console.",
          "Scuro è la pista nera della community: l'immagine della pista diventa nera, così le note, le note lunghe e le linee risaltano.",
          "Sotto il 100 % il locale si vede attraverso la pista delle note: le note, le linee e i bordi restano opachi. 100 % è la pista opaca della console.",
          "Quando sbagli una nota o fai una pennata senza nota, il gioco silenzia la traccia del tuo strumento finché non colpisci di nuovo una nota. Qui scegli quanto ne resta: 0 % la silenzia come sulla console, 50 % la dimezza, 100 % la lascia com'era. La chitarra solista e il basso o la chitarra ritmica; con due giocatori sulla stessa parte (ne condividono la traccia) non cambia.",
      },
      .choice_values = {{{}, {}, {}, {"Originale", "Scuro"}, {}, {}}},
      .done_title = "Il palco è tuo!",
      .done_text = "Il gioco è stato installato. Vai a suonare subito o controlla prima le impostazioni video, grafiche e "
                   "dei tuoi strumenti; potrai sempre cambiarle nel launcher.",
      .dlc_where = "Il gioco mostra i brani in Partita veloce: nella scaletta, con il tasto blu.",
      .extracting_title = "Accordiamo il palco",
      .extracting_text = "Stiamo estraendo il gioco. Può richiedere qualche minuto; non chiudere il launcher.",
      .keyboard_tap_title = "Plettrata alla pressione di un tasto",
      .keyboard_tap_hint = "Ogni tasto della chitarra plettra quando lo premi; i tasti premuti a meno di 20 ms l'uno dall'altro sono un accordo, una sola plettrata.",
  };
  switch (language) {
    case 3:
      return kGerman;
    case 4:
      return kFrench;
    case 5:
      return kSpanish;
    case 6:
      return kItalian;
    default:
      return kEnglish;
  }
}


namespace {

// The frets' colors (the mockup's) and the chips' text colors.
constexpr uint32_t kGreen = 0x3FAE5F;
constexpr uint32_t kRed = 0xD9443A;
constexpr uint32_t kYellow = 0xE8C547;
constexpr uint32_t kBlue = 0x3E82D6;
constexpr uint32_t kOrange = 0xEB8A35;
constexpr uint32_t kGrey = 0x2A2A30;
constexpr uint32_t kOnFret = 0x0E0E10;
constexpr uint32_t kOnGrey = 0xC9C9D0;
constexpr uint32_t kOnStarPower = 0xC5DC3A;

// One language's words for the guitar's functions.
struct ControlWords {
  const char* frets;
  const char* strum_group;
  const char* menus;
  const char* green;
  const char* red;
  const char* yellow;
  const char* blue;
  const char* orange;
  const char* strum_up;
  const char* strum_down;
  const char* whammy;
  const char* whammy_chip;
  const char* dpad_up;
  const char* dpad_down;
  const char* right_stick;
};

// A real guitar's controls, as the game reads them from a guitar: the frets on A, B, Y, X and LB, strumming on the D-pad,
// the whammy bar on the right stick, Star Power on Back. Back is Star Power in the menus too, so it is listed once.
std::vector<GameControlGroup> Guitar(const ControlWords& w) {
  return {
      {w.frets,
       {
           {int(Bind::kA), "", kGreen, kOnFret, w.green},
           {int(Bind::kB), "", kRed, kOnFret, w.red},
           {int(Bind::kY), "", kYellow, kOnFret, w.yellow},
           {int(Bind::kX), "", kBlue, kOnFret, w.blue},
           {int(Bind::kLeftShoulder), "", kOrange, kOnFret, w.orange},
       }},
      {w.strum_group,
       {
           {int(Bind::kDpadUp), "\xE2\x86\x91", kGrey, kOnGrey, w.strum_up, w.dpad_up},
           {int(Bind::kDpadDown), "\xE2\x86\x93", kGrey, kOnGrey, w.strum_down, w.dpad_down},
           {int(Bind::kRStickRight), w.whammy_chip, kGrey, kOnGrey, w.whammy, w.right_stick},
           {int(Bind::kBack), "SP", kGrey, kOnStarPower, "Star Power"},
       }},
      {w.menus,
       {
           {int(Bind::kStart), "START", kGrey, kOnGrey, "Start"},
       }},
  };
}

// One language's words for the game's controller layout.
struct PadWords {
  const char* frets;
  const char* effects;
  const char* menus;
  const char* orange;
  const char* whammy;
  const char* menu_up;
  const char* menu_down;
  const char* back;
  const char* dpad_up;
  const char* dpad_down;
  const char* right_stick;
};

// The game's controller layout (src/startup.cpp's gamepad_subtype = 0; a controller): the frets on LT,
// LB, RB, RT and A, a fret pressed plays its note (no strum); the whammy bar on either stick (listed on the right one);
// Star Power on Back and on the D-pad, which moves in the menus; A selects there, B goes back.
std::vector<GameControlGroup> Pad(const ControlWords& g, const PadWords& w) {
  return {
      {w.frets,
       {
           {int(Bind::kLeftTrigger), "", kGreen, kOnFret, g.green},
           {int(Bind::kLeftShoulder), "", kRed, kOnFret, g.red},
           {int(Bind::kRightShoulder), "", kYellow, kOnFret, g.yellow},
           {int(Bind::kRightTrigger), "", kBlue, kOnFret, g.blue},
           {int(Bind::kA), "", kOrange, kOnFret, w.orange},
       }},
      {w.effects,
       {
           {int(Bind::kRStickRight), g.whammy_chip, kGrey, kOnGrey, g.whammy, w.right_stick},
           {int(Bind::kBack), "SP", kGrey, kOnStarPower, "Star Power"},
       }},
      {w.menus,
       {
           {int(Bind::kDpadUp), "\xE2\x86\x91", kGrey, kOnGrey, w.menu_up, w.dpad_up},
           {int(Bind::kDpadDown), "\xE2\x86\x93", kGrey, kOnGrey, w.menu_down, w.dpad_down},
           {int(Bind::kB), "B", kGrey, kOnGrey, w.back},
           {int(Bind::kStart), "START", kGrey, kOnGrey, "Start"},
       }},
  };
}

// One language's words for the keyboard's functions.
struct KeyWords {
  const char* frets;
  const char* effects;
  const char* menus;
  const char* green;
  const char* red;
  const char* menu_up;
  const char* menu_down;
};

// The keyboard (src/startup.cpp's keyboard_guitar_layout = "gh3", the SDK's change 126): a guitar's keys in the menus,
// the frets on A, B, Y, X and LB (green selects, red goes back, the D-pad moves); in a song the SDK gives the game those
// keys on its controller layout, so a fret key pressed plays (no strum: the D-pad does nothing there), Star Power on
// Back, the whammy bar on the right stick.
std::vector<GameControlGroup> Keyboard(const ControlWords& g, const KeyWords& w) {
  return {
      {w.frets,
       {
           {int(Bind::kA), "", kGreen, kOnFret, w.green},
           {int(Bind::kB), "", kRed, kOnFret, w.red},
           {int(Bind::kY), "", kYellow, kOnFret, g.yellow},
           {int(Bind::kX), "", kBlue, kOnFret, g.blue},
           {int(Bind::kLeftShoulder), "", kOrange, kOnFret, g.orange},
       }},
      {w.effects,
       {
           {int(Bind::kRStickRight), g.whammy_chip, kGrey, kOnGrey, g.whammy, g.right_stick},
           {int(Bind::kBack), "SP", kGrey, kOnStarPower, "Star Power"},
       }},
      {w.menus,
       {
           {int(Bind::kDpadUp), "â", kGrey, kOnGrey, w.menu_up, g.dpad_up},
           {int(Bind::kDpadDown), "â", kGrey, kOnGrey, w.menu_down, g.dpad_down},
           {int(Bind::kStart), "START", kGrey, kOnGrey, "Start"},
       }},
  };
}

}  // namespace

const std::vector<GameControlGroup>& GameKeyboardControlsFor(uint32_t language) {
  static const ControlWords kEnglishWords{"", "", "", "", "", "Yellow fret", "Blue fret", "Orange fret", "", "",
                                          "Whammy bar", "WHAM", "D-pad up", "D-pad down", "Right stick"};
  static const ControlWords kGermanWords{"", "", "", "", "", "Gelber Bund", "Blauer Bund", "Oranger Bund", "", "",
                                         "Whammy-Hebel", "WHAM", "Steuerkreuz hoch", "Steuerkreuz runter",
                                         "Rechter Stick"};
  static const ControlWords kFrenchWords{"", "", "", "", "", "Frette jaune", "Frette bleue", "Frette orange", "", "",
                                         "Levier de vibrato", "VIBR", "Croix haut", "Croix bas", "Stick droit"};
  static const ControlWords kSpanishWords{"", "", "", "", "", "Traste amarillo", "Traste azul", "Traste naranja", "",
                                          "", "Palanca de trémolo", "TRÉM", "Cruceta arriba", "Cruceta abajo",
                                          "Stick derecho"};
  static const ControlWords kItalianWords{"", "", "", "", "", "Tasto giallo", "Tasto blu", "Tasto arancione", "", "",
                                          "Leva del vibrato", "VIBR", "Croce su", "Croce giù", "Levetta destra"};
  static const std::vector<GameControlGroup> kEnglish = Keyboard(
      kEnglishWords, {"Keyboard Â· Frets (a press plays the note)", "Keyboard Â· Whammy and Star Power",
                      "Menus", "Green fret Â· Select", "Red fret Â· Back", "Menu up", "Menu down"});
  static const std::vector<GameControlGroup> kGerman = Keyboard(
      kGermanWords, {"Tastatur Â· Bünde (Drücken spielt die Note)", "Tastatur Â· Whammy und Star Power",
                     "Menüs", "Grüner Bund Â· Auswählen", "Roter Bund Â· Zurück", "Menü hoch",
                     "Menü runter"});
  static const std::vector<GameControlGroup> kFrench = Keyboard(
      kFrenchWords, {"Clavier Â· Frettes (appuyer joue la note)", "Clavier Â· Vibrato et Star Power",
                     "Menus", "Frette verte Â· Choisir", "Frette rouge Â· Retour", "Menu haut",
                     "Menu bas"});
  static const std::vector<GameControlGroup> kSpanish = Keyboard(
      kSpanishWords, {"Teclado Â· Trastes (pulsar toca la nota)", "Teclado Â· Trémolo y Star Power",
                      "Menús", "Traste verde Â· Seleccionar", "Traste rojo Â· Atrás", "Menú arriba",
                      "Menú abajo"});
  static const std::vector<GameControlGroup> kItalian = Keyboard(
      kItalianWords, {"Tastiera Â· Tasti (premere suona la nota)", "Tastiera Â· Vibrato e Star Power",
                      "Menu", "Tasto verde Â· Seleziona", "Tasto rosso Â· Indietro", "Menu su",
                      "Menu giù"});
  switch (language) {
    case 3:
      return kGerman;
    case 4:
      return kFrench;
    case 5:
      return kSpanish;
    case 6:
      return kItalian;
    default:
      return kEnglish;
  }
}

const std::vector<GameControlGroup>& GameGuitarControlsFor(uint32_t language) {
  static const std::vector<GameControlGroup> kEnglish = Guitar({
      "Guitar \xC2\xB7 Frets", "Guitar \xC2\xB7 Strum and effects", "Menus", "Green fret", "Red fret", "Yellow fret",
      "Blue fret", "Orange fret", "Strum up", "Strum down", "Whammy bar", "WHAM", "D-pad up", "D-pad down",
      "Right stick"});
  static const std::vector<GameControlGroup> kGerman = Guitar({
      "Gitarre \xC2\xB7 Bünde", "Gitarre \xC2\xB7 Anschlag und Effekte", "Menüs", "Grüner Bund", "Roter Bund",
      "Gelber Bund", "Blauer Bund", "Oranger Bund", "Anschlag hoch", "Anschlag runter", "Whammy-Hebel", "WHAM",
      "Steuerkreuz hoch", "Steuerkreuz runter", "Rechter Stick"});
  static const std::vector<GameControlGroup> kFrench = Guitar({
      "Guitare \xC2\xB7 Frettes", "Guitare \xC2\xB7 Grattage et effets", "Menus", "Frette verte", "Frette rouge",
      "Frette jaune", "Frette bleue", "Frette orange", "Grattage haut", "Grattage bas", "Levier de vibrato", "VIBR",
      "Croix haut", "Croix bas", "Stick droit"});
  static const std::vector<GameControlGroup> kSpanish = Guitar({
      "Guitarra \xC2\xB7 Trastes", "Guitarra \xC2\xB7 Rasgueo y efectos", "Menús", "Traste verde", "Traste rojo",
      "Traste amarillo", "Traste azul", "Traste naranja", "Rasgueo arriba", "Rasgueo abajo", "Palanca de trémolo",
      "TRÉM", "Cruceta arriba", "Cruceta abajo", "Stick derecho"});
  static const std::vector<GameControlGroup> kItalian = Guitar({
      "Chitarra \xC2\xB7 Tasti", "Chitarra \xC2\xB7 Plettro ed effetti", "Menu", "Tasto verde", "Tasto rosso",
      "Tasto giallo", "Tasto blu", "Tasto arancione", "Plettrata su", "Plettrata giù", "Leva del vibrato", "VIBR",
      "Croce su", "Croce giù", "Levetta destra"});
  switch (language) {
    case 3:
      return kGerman;
    case 4:
      return kFrench;
    case 5:
      return kSpanish;
    case 6:
      return kItalian;
    default:
      return kEnglish;
  }
}

const std::vector<GameControlGroup>& GameControlsFor(uint32_t language) {
  static const ControlWords kEnglishWords{
      "", "", "", "Green fret", "Red fret", "Yellow fret", "Blue fret", "", "", "", "Whammy bar", "WHAM", "", "", ""};
  static const ControlWords kGermanWords{
      "", "", "", "Grüner Bund", "Roter Bund", "Gelber Bund", "Blauer Bund", "", "", "", "Whammy-Hebel", "WHAM", "", "",
      ""};
  static const ControlWords kFrenchWords{
      "", "", "", "Frette verte", "Frette rouge", "Frette jaune", "Frette bleue", "", "", "", "Levier de vibrato", "VIBR",
      "", "", ""};
  static const ControlWords kSpanishWords{
      "", "", "", "Traste verde", "Traste rojo", "Traste amarillo", "Traste azul", "", "", "", "Palanca de trémolo",
      "TRÉM", "", "", ""};
  static const ControlWords kItalianWords{
      "", "", "", "Tasto verde", "Tasto rosso", "Tasto giallo", "Tasto blu", "", "", "", "Leva del vibrato", "VIBR", "",
      "", ""};
  static const std::vector<GameControlGroup> kEnglish = Pad(
      kEnglishWords, {"Controller \xC2\xB7 Frets (a press plays the note)", "Controller \xC2\xB7 Whammy and Star Power",
                      "Menus (the D-pad is Star Power in a song)", "Orange fret \xC2\xB7 Select", "Whammy bar",
                      "Menu up \xC2\xB7 Star Power", "Menu down \xC2\xB7 Star Power", "Back", "D-pad up", "D-pad down",
                      "Right stick"});
  static const std::vector<GameControlGroup> kGerman = Pad(
      kGermanWords, {"Controller \xC2\xB7 Bünde (Drücken spielt die Note)", "Controller \xC2\xB7 Whammy und Star Power",
                     "Menüs (das Steuerkreuz ist im Song Star Power)", "Oranger Bund \xC2\xB7 Auswählen", "Whammy-Hebel",
                     "Menü hoch \xC2\xB7 Star Power", "Menü runter \xC2\xB7 Star Power", "Zurück", "Steuerkreuz hoch",
                     "Steuerkreuz runter", "Rechter Stick"});
  static const std::vector<GameControlGroup> kFrench = Pad(
      kFrenchWords, {"Manette \xC2\xB7 Frettes (appuyer joue la note)", "Manette \xC2\xB7 Vibrato et Star Power",
                     "Menus (la croix est le Star Power en morceau)", "Frette orange \xC2\xB7 Choisir", "Levier de vibrato",
                     "Menu haut \xC2\xB7 Star Power", "Menu bas \xC2\xB7 Star Power", "Retour", "Croix haut", "Croix bas",
                     "Stick droit"});
  static const std::vector<GameControlGroup> kSpanish = Pad(
      kSpanishWords, {"Mando \xC2\xB7 Trastes (pulsar toca la nota)", "Mando \xC2\xB7 Trémolo y Star Power",
                      "Menús (la cruceta es Star Power en una canción)", "Traste naranja \xC2\xB7 Seleccionar",
                      "Palanca de trémolo", "Menú arriba \xC2\xB7 Star Power", "Menú abajo \xC2\xB7 Star Power", "Atrás",
                      "Cruceta arriba", "Cruceta abajo", "Stick derecho"});
  static const std::vector<GameControlGroup> kItalian = Pad(
      kItalianWords, {"Controller \xC2\xB7 Tasti (premere suona la nota)", "Controller \xC2\xB7 Vibrato e Star Power",
                      "Menu (la croce è lo Star Power in un brano)", "Tasto arancione \xC2\xB7 Seleziona",
                      "Leva del vibrato", "Menu su \xC2\xB7 Star Power", "Menu giù \xC2\xB7 Star Power", "Indietro",
                      "Croce su", "Croce giù", "Levetta destra"});
  switch (language) {
    case 3:
      return kGerman;
    case 4:
      return kFrench;
    case 5:
      return kSpanish;
    case 6:
      return kItalian;
    default:
      return kEnglish;
  }
}

}  // namespace launcher::game
