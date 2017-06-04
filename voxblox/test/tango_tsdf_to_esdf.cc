#include <iostream>  // NOLINT

#include "./Block.pb.h"
#include "./Layer.pb.h"
#include "voxblox/core/block.h"
#include "voxblox/core/layer.h"
#include "voxblox/core/voxel.h"
#include "voxblox/io/layer_io.h"
#include "voxblox/test/layer_test_utils.h"

#include <voxblox/io/mesh_ply.h>
#include <voxblox/mesh/mesh_integrator.h>

#include <voxblox/core/esdf_map.h>
#include <voxblox/integrator/esdf_tango_integrator.h>

using namespace voxblox;  // NOLINT

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    throw std::runtime_error("Args: filename to load, followed by filename to save to");
  }

  const std::string file = argv[1];

  Layer<TangoTsdfVoxel>::Ptr layer_from_file;
  io::LoadLayer<TangoTsdfVoxel>(file, &layer_from_file);

  std::cout << "Layer memory size: " << layer_from_file->getMemorySize() << "\n";

  // ESDF maps.
  EsdfMap::Config esdf_config;
  std::shared_ptr<EsdfMap> esdf_map_;
  std::unique_ptr<EsdfTangoIntegrator> esdf_integrator_;

  // Same number of voxels per side for ESDF as with TSDF
  esdf_config.esdf_voxels_per_side = layer_from_file->voxels_per_side();
  esdf_map_.reset(new EsdfMap(esdf_config));
  EsdfTangoIntegrator::Config esdf_integrator_config;
  // Make sure that this is the same as the truncation distance OR SMALLER!
  esdf_integrator_config.min_distance_m = esdf_config.esdf_voxel_size;
  // esdf_integrator_config.min_distance_m =
  //    tsdf_integrator_->getConfig().default_truncation_distance;
  esdf_integrator_.reset(new EsdfTangoIntegrator(esdf_integrator_config,
                                                 layer_from_file.get(),
                                                 esdf_map_->getEsdfLayerPtr()));
  esdf_integrator_->updateFromTsdfLayerBatch();

  io::SaveLayer(esdf_map_->getEsdfLayer(), argv[2]);

  return 0;
}
