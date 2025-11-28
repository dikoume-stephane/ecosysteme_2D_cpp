#include "Core/Entity.h" 
#include <cmath> 
#include <iostream> 
#include <algorithm> 
namespace Ecosystem
{ 
    namespace Core
    { 
        // 🏗 CONSTRUCTEUR PRINCIPAL 
        Entity::Entity(EntityType type, Vector2D pos, std::string entityName) 
            : mType(type), position(pos), name(entityName),  
            mRandomGenerator(std::random_device{}())  // Initialisation du générateur alé
        { 
            // INITIALISATION SELON LE TYPE 
            switch(mType)
            { 
                case EntityType::HERBIVORE: 
                    mEnergy = 80.0f; 
                    mMaxEnergy = 150.0f; 
                    mMaxAge = 200; 
                    color = Color::Blue(); 
                    size = 8.0f; 
                    break; 
                case EntityType::CARNIVORE: 
                    mEnergy = 100.0f; 
                    mMaxEnergy = 200.0f; 
                    mMaxAge = 150; 
                    color = Color::Red(); 
                    size = 12.0f; 
                    break; 
                case EntityType::PLANT: 
                    mEnergy = 50.0f; 
                    mMaxEnergy = 100.0f; 
                    mMaxAge = 300; 
                    color = Color::Green(); 
                    size = 6.0f; 
                    break; 
            }
            mAge = 0; 
            mIsAlive = true; 
            mVelocity = GenerateRandomDirection(); 
            std::cout << "🌱Entité créée: " << name << " à (" << position.x << ", " << position.y<<")"<< std::endl;
        } 

        // 🏗 CONSTRUCTEUR DE COPIE 
        Entity::Entity(const Entity& other) 
            : mType(other.mType), position(other.position), name(other.name + "_copy"), 
            mEnergy(other.mEnergy * 0.7f),  // Enfant a moins d'énergie 
            mMaxEnergy(other.mMaxEnergy), 
            mAge(0),  // Nouvelle entité, âge remis à 0 
            mMaxAge(other.mMaxAge), 
            mIsAlive(true), 
            mVelocity(other.mVelocity), 
            color(other.color), 
            size(other.size * 0.8f),  // Enfant plus petit 
            mRandomGenerator(std::random_device{}()) 
        { 
            std::cout << "👶Copie d'entité créée: " << name << std::endl; 
        } 
            
        // 🗑 DESTRUCTEUR 
        Entity::~Entity()
        { 
            std::cout << "💀Entité détruite: " << name << " (Âge: " << mAge << ")" << std::endl; 
        } 

        //⚙MISE À JOUR PRINCIPALE 
        void Entity::Update(float deltaTime, const std::vector<Food>& foodsource, const std::vector<std::unique_ptr<Entity>>& allentities)
        { 
            if (!mIsAlive) return; 
            // PROCESSUS DE VIE 
            ConsumeEnergy(deltaTime); 
            Age(deltaTime); 
            Move(deltaTime, foodsource, allentities); 
            CheckVitality(); 
        } 

        void ApplyForce(Vector2D force)
        {

        }; 

        // MOUVEMENT 
        void Entity::Move(float deltaTime, const std::vector<Food>& foodsource, const std::vector<std::unique_ptr<Entity>>& allentities)
        { 
            
            if (mType == EntityType::PLANT) return;  // Les plantes ne bougent pas 
            // Comportement aléatoire occasionnel 
            std::uniform_real_distribution<float> chance(0.0f, 1.0f); 
            if (chance(mRandomGenerator) < 0.02f) { 
                mVelocity = GenerateRandomDirection(); 
            }
            // Application du mouvement 
            if (mType==EntityType::HERBIVORE) position = position.operator+(AvoidPredators(allentities));
            /*if(mEnergy<(mMaxEnergy*0.5f))
            {
                switch(mType)
                {
                case EntityType::CARNIVORE:
                position = position.operator+(SeekFood(allentities));
                break;
                case EntityType::HERBIVORE:
                position = position.operator+(SeekFood(foodsource));
                break;
                }
                
            }*/
            
            position =StayInBounds(  1200.0f, 800.0f) ;
            position = position + mVelocity * deltaTime * 20.0f; 
            // Consommation d'énergie due au mouvement 
            mEnergy -= mVelocity.Distance(Vector2D(0, 0)) * deltaTime * 0.1f; 
        } 

        // 🍽 MANGER
        void Entity::Eat(float energy) 
        { 
            mEnergy += energy; 
            if (mEnergy > mMaxEnergy)
            { 
                mEnergy = mMaxEnergy; 
            } 
            std::cout << "🍽 " << name << " mange et gagne " << energy << " énergie" << std::endl;
        }
            
        // CONSOMMATION D'ÉNERGIE 
        void Entity::ConsumeEnergy(float deltaTime)
        { 
            float baseConsumption = 0.0f; 
            switch(mType)
            { 
                case EntityType::HERBIVORE: 
                    baseConsumption = 1.5f; 
                    break; 
                case EntityType::CARNIVORE: 
                    baseConsumption = 2.0f; 
                    break; 
                case EntityType::PLANT: 
                    baseConsumption = -0.5f;  // Les plantes génèrent de l'énergie ! 
                    break; 
            }
            mEnergy -= baseConsumption * deltaTime; 
        } 

        // MÉTHODES DE COMPORTEMENT 
       Vector2D Entity::SeekFood(const std::vector<Food>& foodSources) const
        {

            if(foodSources.empty())
            {
                Vector2D rien={0.f,0.f};
                return rien;
            } 
            Vector2D trcfood;
            float mindist=9900.f,distvision =230.f,dist=0.f;
        
            for (Food foodSource : foodSources)
            { 
                dist =position.Distance( foodSource.position);
                if (mindist>dist && dist<distvision)
                {
                    mindist =dist;
                    trcfood =foodSource.position;
                }
            }
            if( mindist==9900.f)
            {
                return {0.f,0.f};
            }
            trcfood ={trcfood.x-position.x,trcfood.y-position.y};
            float norm =sqrt(trcfood.x*trcfood.x+trcfood.y*trcfood.y);

            Vector2D direction={trcfood.x/norm,trcfood.y/norm};
            direction =direction.operator*(0.25f);
            return direction;
        }

        Vector2D Entity::SeekFood(const std::vector<std::unique_ptr<Entity>>& entityfood) const
        {

            if(entityfood.empty())
            {
                Vector2D rien={0,0};
                return rien;
            }
            
            Vector2D trcfood;
            float mindist=9900.f,distvision =330.f,dist=0.f;
            
        
            for (const std::unique_ptr<Entity>& foodSource : entityfood)
            {
                const Entity& entfood = *foodSource;
                if (entfood.mType ==EntityType::HERBIVORE)
                {
                    dist=position.Distance( entfood.position);
                    if (mindist>dist && dist<distvision)
                    {
                        mindist =dist;
                        trcfood =entfood.position;
                    }
                }
            }

            if( mindist==9900.f)
            {
                return {0.f,0.f};
            }
            trcfood ={trcfood.x-position.x,trcfood.y-position.y};
            float norm =sqrt(trcfood.x*trcfood.x+trcfood.y*trcfood.y);

            Vector2D direction={trcfood.x/norm,trcfood.y/norm};
            direction =direction.operator*(0.25f);
            return direction;
        }

        Vector2D Entity::AvoidPredators(const std::vector<std::unique_ptr<Entity>>& predators) const
        {
            Vector2D fuit,observ;
            float dis;
            for (const std::unique_ptr<Entity>& predator: predators)
            {
                const Entity& predat =*predator;
                if (mType !=predat.mType) continue;
                observ ={position.x-predat.position.x , position.y-predat.position.y};
                dis =sqrt( observ.x*observ.x + observ.y*observ.y);
                if (dis<100.0f && dis>0.01f)
                {
                    fuit =fuit.operator+({observ.x/(dis*dis), observ.y/(dis*dis)});
                }
            }
            if(fuit.x==0.0f && fuit.y==0.0f)
            {
                return fuit;
            }

            float norm =sqrt(fuit.x*fuit.x+fuit.y*fuit.y);
            fuit ={fuit.x/norm, fuit.y/norm};
            fuit =fuit.operator*(0.25f);
            return fuit;
        }

        Vector2D Entity::StayInBounds(float worldWidth, float worldHeight) 
        {
            if(position.x<0) position.x =0;
            if(position.y<0) position.y =0;
            if(position.x>worldWidth) position.x =worldWidth;
            if(position.y>worldHeight) position.y =worldHeight;
            return position;
        } 

        // VIEILLISSEMENT 
        void Entity::Age(float deltaTime)
        { 
            mAge += static_cast<int>(deltaTime * 10.0f);  // Accéléré pour la simulation 
        } 

        // ❤VÉRIFICATION DE LA SANTÉ 
        void Entity::CheckVitality()
        { 
            if (mEnergy <= 0.0f || mAge >= mMaxAge)
            { 
                mIsAlive = false; 
                std::cout << "💀" << name << " meurt - "; 
                if (mEnergy <= 0) std::cout << "Faim"; 
                else std::cout << "Vieillesse"; 
                std::cout << std::endl; 
            }
        } 

        // REPRODUCTION 
        bool Entity::CanReproduce() const
        { 
            return mIsAlive && mEnergy > mMaxEnergy * 0.8f && mAge > 20; 
        } 

        std::unique_ptr<Entity> Entity::Reproduce()
        { 
            if (!CanReproduce()) return nullptr; 
            // Chance de reproduction 
            std::uniform_real_distribution<float> chance(0.0f, 1.0f); 
            if (chance(mRandomGenerator) < 0.3f) { 
            }
                mEnergy *= 0.6f;  // Coût énergétique de la reproduction 
                return std::make_unique<Entity>(*this);  // Utilise le constructeur de copi
            return nullptr; 
        } 

        // GÉNÉRATION DE DIRECTION ALÉATOIRE 
        Vector2D Entity::GenerateRandomDirection()
        { 
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f); 
            return Vector2D(dist(mRandomGenerator), dist(mRandomGenerator)); 
        } 

        // CALCUL DE LA COULEUR BASÉE SUR L'ÉTAT 
        Color Entity::CalculateColorBasedOnState() const
        { 
            float energyRatio = GetEnergyPercentage(); 
            Color baseColor = color; 
            // Rouge si faible énergie 
            if (energyRatio < 0.3f) { 
                baseColor.r = 255; 
                baseColor.g = static_cast<uint8_t>(baseColor.g * energyRatio); 
                baseColor.b = static_cast<uint8_t>(baseColor.b * energyRatio); 
            }
            return baseColor; 
        } 

        //RENDU GRAPHIQUE 
        void Entity::Render(SDL_Renderer* renderer) const
        { 
            if (!mIsAlive) return; 
            Color renderColor = CalculateColorBasedOnState(); 
            SDL_FRect rect = { 
                position.x - size / 2.0f, 
                position.y - size / 2.0f, 
                size, 
                size 
            }; 
            SDL_SetRenderDrawColor(renderer, renderColor.r, renderColor.g, renderColor.b, renderColor.a);
            SDL_RenderFillRect(renderer, &rect); 
            // Indicateur d'énergie (barre de vie) 
            if (mType != EntityType::PLANT)
            { 
                float energyBarWidth = size * GetEnergyPercentage(); 
                SDL_FRect energyBar = { 
                    position.x - size / 2.0f, 
                    position.y - size / 2.0f - 3.0f, 
                    energyBarWidth, 
                    2.0f 
                };
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
                SDL_RenderFillRect(renderer, &energyBar); 
            }
        } 
    } // namespace Core 
} // namespace Ecosystem