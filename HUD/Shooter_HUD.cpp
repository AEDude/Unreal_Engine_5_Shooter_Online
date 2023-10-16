// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_HUD.h"

void AShooter_HUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D Viewport_Size;
    if(GEngine)
    {
        GEngine->GameViewport->GetViewportSize(Viewport_Size);
        const FVector2D Viewport_Center(Viewport_Size.X / 2.f, Viewport_Size.Y / 2.f);

        float Spread_Scaled = Crosshiar_Spread_Max * HUD_Package.Crosshair_Spread;

        if(HUD_Package.Crosshair_Center)
        {
            FVector2D Spread(0.f, 0.f);
            Draw_Crosshair(HUD_Package.Crosshair_Center, Viewport_Center, Spread, HUD_Package.Crosshairs_Color);
        }
        if(HUD_Package.Crosshair_Top)
        {
            FVector2D Spread(0.f, -Spread_Scaled);
            Draw_Crosshair(HUD_Package.Crosshair_Top, Viewport_Center, Spread, HUD_Package.Crosshairs_Color);
        }
        if(HUD_Package.Crosshair_Bottom)
        {
            FVector2D Spread(0.f, Spread_Scaled);
            Draw_Crosshair(HUD_Package.Crosshair_Bottom, Viewport_Center, Spread, HUD_Package.Crosshairs_Color);
        }
        if(HUD_Package.Crosshair_Left)
        {
            FVector2D Spread(-Spread_Scaled, 0.f);
            Draw_Crosshair(HUD_Package.Crosshair_Left, Viewport_Center, Spread, HUD_Package.Crosshairs_Color);
        }
        if(HUD_Package.Crosshair_Right)
        {
            FVector2D Spread(Spread_Scaled, 0.f);
            Draw_Crosshair(HUD_Package.Crosshair_Right, Viewport_Center, Spread, HUD_Package.Crosshairs_Color);
        }
    }
}

void AShooter_HUD::Draw_Crosshair(UTexture2D *Texture, FVector2D Viewport_Center, FVector2D Spread, FLinearColor Crosshair_Color)
{
    const float Texture_Width = Texture->GetSizeX();
    const float Texture_Height = Texture->GetSizeY();
    const FVector2D Texture_Draw_Point(
        Viewport_Center.X - (Texture_Width / 2.f) + Spread.X,
        Viewport_Center.Y - (Texture_Height / 2.f) + Spread.Y
    );

    DrawTexture(
        Texture,
        Texture_Draw_Point.X,
        Texture_Draw_Point.Y,
        Texture_Height,
        Texture_Height,
        0.f,
        0.f,
        1.f,
        1.f,
        Crosshair_Color
    );
}
