# 💎 Prism UI : Guide du Développeur

**Prism UI** est un framework d'interface utilisateur C++ de classe AAA pour Unreal Engine 5. Il a été conçu pour être **strictement "zero-tick"** (piloté par des événements), **orienté Data (Tokens Sémantiques)**, et **extrêmement rapide à utiliser** pour les programmeurs grâce à l'approche *Fluent Builder*.

---

## 1. Philosophie et Architecture Centrale

### Zéro-Tick & Performances
Dans Prism UI, aucun widget ne doit *tick* par défaut. Les mises à jour visuelles (changements de thème, survol de souris) déclenchent des timers temporaires (`StartVisualTransition`) qui s'éteignent automatiquement dès que l'animation est terminée.

### Tokens Sémantiques
Finies les couleurs codées en dur ! Prism utilise un système de tokens (`EPrismColorToken` et `EPrismTypographyToken`). Au lieu de demander "du bleu", vous demandez la couleur `PrimaryAccent`. Si le joueur passe en "Mode Sombre" ou "Thème Rouge", **toute l'interface du jeu se mettra à jour instantanément sans aucun code supplémentaire**.

---

## 2. Le Sous-système de Thème (`UPrismThemeSubsystem`)

Le cœur du système est le `UPrismThemeSubsystem` (un `ULocalPlayerSubsystem`). Il contient la donnée `UPrismUIThemeData` actuellement active pour le joueur.

> [!TIP]
> **Pas besoin de vous y abonner manuellement !** 
> Si votre widget hérite de `UPrismWidgetBase`, il s'abonne et se désabonne automatiquement du sous-système lors de sa création/destruction.

### Définir le Rôle de votre Widget
Chaque widget Prism possède une variable `WidgetRole` (ex: `"Primary"`, `"Danger"`, `"Tactical"`). Ce rôle permet au sous-système de savoir quelle "Palette" (`UPrismUIStyle`) appliquer à ce widget précis.

---

## 3. Créer un Widget C++ avec `UPrismWidgetBase`

Oubliez les constructeurs lourds. Pour créer un widget Prism :

1. Faites hériter votre classe de `UPrismWidgetBase`.
2. Surchargez **uniquement** la fonction `BuildDefaultLayout()`.
3. Surchargez `OnVisualsUpdated()` si vous voulez animer des éléments au survol.

```cpp
UCLASS()
class UMyCustomPanel : public UPrismWidgetBase
{
	GENERATED_BODY()

protected:
	// C'est ici que vous construisez l'arborescence UI
	virtual void BuildDefaultLayout() override;

	// Optionnel : Pour réagir aux couleurs du thème
	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData) override;
};
```

---

## 4. Construire l'UI avec le `FPrismBuilder` (Super Pouvoir ⚡)

La plus grande force de Prism UI pour les développeurs est le `FPrismBuilder`. Inspiré des frameworks web et de *Slate*, il vous permet de générer des arborescences UMG complexes en quelques lignes de code chaînées, sans jamais manipuler de pointeurs inutiles.

> [!IMPORTANT]
> Le `FPrismBuilder` applique **automatiquement** les couleurs et polices de votre thème !

### Exemple de Création d'une Carte (Card)
Dans votre fonction `BuildDefaultLayout()`, utilisez le Builder :

```cpp
#include "Utilities/PrismBuilder.h"

void UMyCustomPanel::BuildDefaultLayout()
{
	Super::BuildDefaultLayout();

	FPrismBuilder Builder(this, WidgetTree);

	Builder.BeginOverlay() // Racine de la carte
		
		// 1. Un fond sombre avec contour
		.AddBackground("Primary", EPrismColorToken::BackgroundDeep)
		
		// 2. Le contenu vertical
		.BeginVerticalBox()
			.SetPadding(FMargin(20.0f))

			// Titre
			.AddText(FText::FromString("PARAMÈTRES SYSTÈME"), "Primary", EPrismTypographyToken::H1)
			
			// Ligne de séparation (Bordure fine)
			.AddBackground("Primary", EPrismColorToken::Outline)
			.SetSize(FSlateChildSize(ESlateSizeRule::Automatic))
			
			// Sous-contenu
			.BeginHorizontalBox()
				.SetPadding(FMargin(0, 10.f, 0, 0))
				.AddText(FText::FromString("Statut : "), "Primary", EPrismTypographyToken::Body, EPrismColorToken::TextSecondary)
				.AddText(FText::FromString("EN LIGNE"), "Primary", EPrismTypographyToken::Body, EPrismColorToken::Success)
			.EndPanel() // Fin du HorizontalBox

		.EndPanel() // Fin du VerticalBox

	.EndPanel(); // Fin de l'Overlay

	Builder.Build();
}
```

### Méthodes clés du Builder :
- `BeginVerticalBox()`, `BeginHorizontalBox()`, `BeginOverlay()` : Ouvre un nouveau conteneur. Tout ce qui suit ira dedans.
- `EndPanel()` : Ferme le conteneur actuel et remonte d'un niveau dans la hiérarchie.
- `AddBackground(Role, ColorToken)` : Ajoute un `UBorder` coloré via le thème.
- `AddText(Text, Role, TypoToken, ColorToken)` : Ajoute un `UTextBlock` formaté via le thème.
- `SetPadding()`, `SetHorizontalAlignment()`, `SetSize()` : Modifie la marge/l'alignement **du dernier élément ajouté**.

---

## 5. Data Binding Zéro-Tick (`BindEvent`)

Au lieu d'utiliser le Property Binding très lourd d'Unreal (basé sur la Reflection) ou de scanner des valeurs dans `NativeTick`, Prism introduit un Data Binding réactif intégré directement au Builder.

Vous pouvez lier un événement C++ (Multicast Delegate) au moment même de la création de votre composant :

```cpp
Builder.BeginVerticalBox()
	.AddText(FText::FromString("100"), "Primary", EPrismTypographyToken::H1)
	
	// S'abonne à l'événement du joueur de façon totalement sécurisée !
	.BindEvent(PlayerRef->OnHealthChanged, [](UWidget* Target, int32 NewHP)
	{
		if (UTextBlock* Text = Cast<UTextBlock>(Target))
		{
			Text->SetText(FText::AsNumber(NewHP));
		}
	});
```

> [!TIP]
> **Performance Garantie** : Si votre widget est détruit ou rangé dans le Pool, la liaison mémoire `BindEvent` est instantanément court-circuitée pour éviter toute consommation de CPU inutile ou risque de crash.

---

## 6. Optimisation Avancée : Le Widget Pool

Instancier (`CreateWidget`) et détruire des widgets constamment génère de la fragmentation mémoire et des ralentissements dus au Garbage Collection. Prism UI intègre un **Subsystem de Pooling** très performant (`UPrismUIWidgetPoolSubsystem`) propre à chaque joueur local.

`UPrismWidgetBase` implémente nativement `IPrismPoolableWidget`. Il gère donc de manière autonome son nettoyage (désactivation des timers, effacement des Data Bindings) lorsqu'il retourne dans le pool !

**Comment utiliser le pool ?**

```cpp
// Au lieu de CreateWidget...
if (UPrismUIWidgetPoolSubsystem* Pool = GetOwningLocalPlayer()->GetSubsystem<UPrismUIWidgetPoolSubsystem>())
{
	// 1. Récupérer un widget du pool (ou le créer s'il est vide)
	UUserWidget* MyWidget = Pool->AcquireWidget(UMyCustomPanel::StaticClass());
	
	// ... l'utiliser ...
	
	// 2. Quand vous n'en avez plus besoin (le widget disparaît sans être détruit)
	Pool->ReleaseWidget(MyWidget);
}
```

---

## 7. Animations et États Interactifs

Prism UI gère l'état d'interaction de votre widget via `EPrismWidgetState` (Normal, Hovered, Pressed, Selected, Disabled).

Si vous créez un bouton interactif, surchargez `OnStateChanged` et appelez `StartVisualTransition` :

```cpp
void UMyCustomButton::OnStateChanged(EPrismWidgetState InNewState)
{
	Super::OnStateChanged(InNewState);

	if (InNewState == EPrismWidgetState::Hovered)
	{
		// Cible un Alpha de 1.0f à une vitesse de 20.0
		StartVisualTransition(1.0f, 20.0f); 
	}
	else
	{
		// Retourne à un Alpha de 0.0f à une vitesse de 10.0
		StartVisualTransition(0.0f, 10.0f);
	}
}
```

Ensuite, surchargez `OnVisualsUpdated(float InBlendValue)` (qui tick de manière sûre uniquement pendant l'animation) :

```cpp
void UMyCustomButton::OnVisualsUpdated(float InBlendValue)
{
	Super::OnVisualsUpdated(InBlendValue);

	// InBlendValue va de 0.0f (Normal) à 1.0f (Hovered) de manière fluide !
	const float Scale = FMath::Lerp(1.0f, 1.05f, InBlendValue);
	SetRenderScale(FVector2D(Scale, Scale));

	// Vous pouvez aussi interpoler des couleurs, des marges, etc.
}
```

---

## En Résumé pour l'équipe
1. Ne **codez plus aucune couleur en dur** (`FLinearColor`). Utilisez `EPrismColorToken`.
2. Oubliez `CreateWidget<UTextBlock>`. Utilisez le `FPrismBuilder` pour un code lisible de haut en bas.
3. Ne **bindez jamais** vos données dans `NativeTick`. Utilisez `.BindEvent()` via le Builder.
4. Pour des listes ou des éléments fréquents, utilisez **toujours** le `UPrismUIWidgetPoolSubsystem`.
5. Évitez d'utiliser `NativeTick`. Utilisez `StartVisualTransition` pour tout mouvement fluide d'interface.
6. Laissez `PrismWidgetBase` s'occuper de la mise à jour des thèmes à votre place !
