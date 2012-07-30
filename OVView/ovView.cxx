#include "ovView.h"
#include "ovGLContext.h"

#include "QVTKInteractor.h"
#include "vtkContextScene.h"
#include "vtkContextTransform.h"
#include "vtkContextView.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkInteractorObserver.h"
#include "vtkNew.h"
#include "vtkBlockItem.h"

#include "vtkRandomGraphSource.h"
#include "vtkGraphLayoutView.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRendererCollection.h"

ovView::ovView(QGraphicsItem *p)
  : QVTKGraphicsItem(ovGLContext::instance(), p)
{
#if 0
  vtkNew<vtkConeSource> source;
  vtkNew<vtkPolyDataMapper> mapper;
  vtkNew<vtkActor> actor;
  mapper->SetInputConnection(source->GetOutputPort());
  actor->SetMapper(mapper.GetPointer());
  vtkNew<vtkRenderer> ren;
  ren->AddActor(actor.GetPointer());
  this->GetRenderWindow()->AddRenderer(ren.GetPointer());
#endif
#if 0
  vtkNew<vtkRandomGraphSource> source;
  source->SetNumberOfVertices(100);
  source->SetStartWithTree(true);
  source->SetNumberOfEdges(0);
  this->View->AddRepresentationFromInputConnection(source->GetOutputPort());
#endif

  //this->GetInteractor()->SetInteractorStyle(this->View->GetInteractor()->GetInteractorStyle());
  //this->GetInteractor()->Initialize();
  this->View->SetRenderWindow(this->GetRenderWindow());
  std::cerr << this->View->GetInteractor()->GetClassName() << std::endl;
  std::cerr << this->View->GetInteractor()->GetInteractorStyle()->GetClassName() << std::endl;
  //this->View->SetInteractor(this->GetInteractor());
  vtkNew<vtkBlockItem> block;
  block->SetDimensions(200, 200, 100, 100);
  vtkNew<vtkContextTransform> trans;
  trans->SetInteractive(true);
  trans->AddItem(block.GetPointer());
  this->View->GetScene()->AddItem(trans.GetPointer());
#if 0

  QFile f1(":/Data/treetest.xml");
  f1.open(QIODevice::ReadOnly);
  QByteArray f1_data = f1.readAll();

  vtkSmartPointer<vtkXMLTreeReader> reader = vtkSmartPointer<vtkXMLTreeReader>::New();
  reader->SetXMLString(f1_data.data());
  reader->SetMaskArrays(true);
  reader->Update();
  vtkTree* t = reader->GetOutput();
  vtkSmartPointer<vtkStringArray> label = vtkSmartPointer<vtkStringArray>::New();
  label->SetName("edge label");
  vtkSmartPointer<vtkIdTypeArray> dist = vtkSmartPointer<vtkIdTypeArray>::New();
  dist->SetName("distance");
  for (vtkIdType i = 0; i < t->GetNumberOfEdges(); i++)
  {
    dist->InsertNextValue(i);
    switch (i % 3)
    {
      case 0:
        label->InsertNextValue("a");
        break;
      case 1:
        label->InsertNextValue("b");
        break;
      case 2:
        label->InsertNextValue("c");
        break;
    }
  }
  t->GetEdgeData()->AddArray(dist);
  t->GetEdgeData()->AddArray(label);

  vtkSmartPointer<vtkStringToNumeric> numeric = vtkSmartPointer<vtkStringToNumeric>::New();
  numeric->SetInput(t);

  GraphLayoutView->DisplayHoverTextOn();
  GraphLayoutView->SetLayoutStrategyToCircular();
  GraphLayoutView->SetVertexLabelArrayName("name");
  GraphLayoutView->VertexLabelVisibilityOn();
  GraphLayoutView->SetVertexColorArrayName("size");
  GraphLayoutView->ColorVerticesOn();
  GraphLayoutView->SetRepresentationFromInputConnection(numeric->GetOutputPort());
  GraphLayoutView->SetEdgeColorArrayName("distance");
  GraphLayoutView->ColorEdgesOn();
  GraphLayoutView->SetEdgeLabelArrayName("edge label");
  GraphLayoutView->EdgeLabelVisibilityOn();
  vtkRenderedGraphRepresentation* rep =
    vtkRenderedGraphRepresentation::SafeDownCast(GraphLayoutView->GetRepresentation());
  rep->SetVertexHoverArrayName("name");
  rep->SetEdgeHoverArrayName("edge label");

  GraphLayoutView->SetHideVertexLabelsOnInteraction(1);
  GraphLayoutView->SetHideEdgeLabelsOnInteraction(1);

  GraphLayoutView->ResetCamera();
#endif
}

ovView::~ovView()
{
}
