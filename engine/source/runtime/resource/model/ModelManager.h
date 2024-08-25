

namespace DynastyEngine 
{
    enum class EditMode
    {
        Select = 0,
        Landscape = 1,
        Foliage = 2,
        BrushEditing = 3,
        MeshPaint = 4
    };

    enum class SceneMode
    {
        None = 0,
        EnvironmentHdr = 1,
        SkyBox = 2,
    };

    class ModelManager
    {
    public:
        ModelManager(/* args */);
        ~ModelManager();
    
    private:
        /* data */
        static int b3DModel; // 0 for 2D, 1 for 3D
        static bool bShowPhysicsColliders;
    };
}