/**
  ******************************************************************************
  * @file    model_test1.c
  * @author  AST Embedded Analytics Research Platform
  * @date    Sat Oct 21 02:09:30 2023
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */


#include "model_test1.h"
#include "model_test1_data.h"

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_math_helpers.h"

#include "core_common.h"
#include "core_convert.h"

#include "layers.h"



#undef AI_NET_OBJ_INSTANCE
#define AI_NET_OBJ_INSTANCE g_model_test1
 
#undef AI_MODEL_TEST1_MODEL_SIGNATURE
#define AI_MODEL_TEST1_MODEL_SIGNATURE     "3b8ce54c1a2845a57ad45ed40af9fbe7"

#ifndef AI_TOOLS_REVISION_ID
#define AI_TOOLS_REVISION_ID     ""
#endif

#undef AI_TOOLS_DATE_TIME
#define AI_TOOLS_DATE_TIME   "Sat Oct 21 02:09:30 2023"

#undef AI_TOOLS_COMPILE_TIME
#define AI_TOOLS_COMPILE_TIME    __DATE__ " " __TIME__

#undef AI_MODEL_TEST1_N_BATCHES
#define AI_MODEL_TEST1_N_BATCHES         (1)

static ai_ptr g_model_test1_activations_map[1] = AI_C_ARRAY_INIT;
static ai_ptr g_model_test1_weights_map[1] = AI_C_ARRAY_INIT;



/**  Array declarations section  **********************************************/
/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_1_conv2d_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  dense_1_dense_weights_array, AI_ARRAY_FORMAT_LUT4_FLOAT,
  NULL, NULL, 720896, AI_STATIC)
/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  dense_1_dense_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  input_0_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 270, AI_STATIC)
/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_1_conv2d_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22784, AI_STATIC)
/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  dense_2_dense_weights_array, AI_ARRAY_FORMAT_LUT4_FLOAT,
  NULL, NULL, 16384, AI_STATIC)
/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_1_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 22784, AI_STATIC)
/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  dense_2_dense_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  max_pooling2d_1_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 5632, AI_STATIC)
/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  dense_1_dense_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  dense_3_dense_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 768, AI_STATIC)
/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  dense_1_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  dense_3_dense_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6, AI_STATIC)
/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  dense_2_dense_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  dense_2_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 128, AI_STATIC)
/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  dense_3_dense_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6, AI_STATIC)
/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  dense_3_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 6, AI_STATIC)
/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_1_conv2d_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 512, AI_STATIC)
/**  Tensor declarations section  *********************************************/
/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_1_conv2d_bias, AI_STATIC,
  0, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &conv2d_1_conv2d_bias_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  dense_1_dense_weights, AI_STATIC,
  1, 0x0,
  AI_SHAPE_INIT(4, 5632, 128, 1, 1), AI_STRIDE_INIT(4, 1, 2816, 360448, 360448),
  1, &dense_1_dense_weights_array, NULL)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  dense_1_dense_bias, AI_STATIC,
  2, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_1_dense_bias_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  input_0_output, AI_STATIC,
  3, 0x0,
  AI_SHAPE_INIT(4, 1, 1, 3, 90), AI_STRIDE_INIT(4, 4, 4, 4, 12),
  1, &input_0_output_array, NULL)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_1_conv2d_output, AI_STATIC,
  4, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 2, 89), AI_STRIDE_INIT(4, 4, 4, 512, 1024),
  1, &conv2d_1_conv2d_output_array, NULL)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  dense_2_dense_weights, AI_STATIC,
  5, 0x0,
  AI_SHAPE_INIT(4, 128, 128, 1, 1), AI_STRIDE_INIT(4, 1, 64, 8192, 8192),
  1, &dense_2_dense_weights_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_1_output, AI_STATIC,
  6, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 2, 89), AI_STRIDE_INIT(4, 4, 4, 512, 1024),
  1, &conv2d_1_output_array, NULL)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  dense_2_dense_bias, AI_STATIC,
  7, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_2_dense_bias_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  max_pooling2d_1_output, AI_STATIC,
  8, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 44), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &max_pooling2d_1_output_array, NULL)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  max_pooling2d_1_output0, AI_STATIC,
  9, 0x0,
  AI_SHAPE_INIT(4, 1, 5632, 1, 1), AI_STRIDE_INIT(4, 4, 4, 22528, 22528),
  1, &max_pooling2d_1_output_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  dense_1_dense_output, AI_STATIC,
  10, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_1_dense_output_array, NULL)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  dense_3_dense_weights, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 128, 6, 1, 1), AI_STRIDE_INIT(4, 4, 512, 3072, 3072),
  1, &dense_3_dense_weights_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  dense_1_output, AI_STATIC,
  12, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_1_output_array, NULL)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  dense_3_dense_bias, AI_STATIC,
  13, 0x0,
  AI_SHAPE_INIT(4, 1, 6, 1, 1), AI_STRIDE_INIT(4, 4, 4, 24, 24),
  1, &dense_3_dense_bias_array, NULL)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  dense_2_dense_output, AI_STATIC,
  14, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_2_dense_output_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  dense_2_output, AI_STATIC,
  15, 0x0,
  AI_SHAPE_INIT(4, 1, 128, 1, 1), AI_STRIDE_INIT(4, 4, 4, 512, 512),
  1, &dense_2_output_array, NULL)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  dense_3_dense_output, AI_STATIC,
  16, 0x0,
  AI_SHAPE_INIT(4, 1, 6, 1, 1), AI_STRIDE_INIT(4, 4, 4, 24, 24),
  1, &dense_3_dense_output_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  dense_3_output, AI_STATIC,
  17, 0x0,
  AI_SHAPE_INIT(4, 1, 6, 1, 1), AI_STRIDE_INIT(4, 4, 4, 24, 24),
  1, &dense_3_output_array, NULL)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_1_conv2d_weights, AI_STATIC,
  18, 0x0,
  AI_SHAPE_INIT(4, 1, 2, 2, 128), AI_STRIDE_INIT(4, 4, 4, 512, 1024),
  1, &conv2d_1_conv2d_weights_array, NULL)



/**  Layer declarations section  **********************************************/


AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_3_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_3_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_3_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_3_layer, 6,
  NL_TYPE, 0x0, NULL,
  nl, forward_sm,
  &dense_3_chain,
  NULL, &dense_3_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_3_dense_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_2_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_3_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &dense_3_dense_weights, &dense_3_dense_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_3_dense_layer, 6,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &dense_3_dense_chain,
  NULL, &dense_3_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_2_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_2_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_2_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_2_layer, 5,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &dense_2_chain,
  NULL, &dense_3_dense_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_2_dense_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_1_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_2_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &dense_2_dense_weights, &dense_2_dense_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_2_dense_layer, 5,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &dense_2_dense_chain,
  NULL, &dense_2_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_1_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_1_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_1_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_1_layer, 4,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &dense_1_chain,
  NULL, &dense_2_dense_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  dense_1_dense_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &max_pooling2d_1_output0),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &dense_1_dense_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &dense_1_dense_weights, &dense_1_dense_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  dense_1_dense_layer, 4,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &dense_1_dense_chain,
  NULL, &dense_1_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  max_pooling2d_1_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_1_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &max_pooling2d_1_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  max_pooling2d_1_layer, 1,
  POOL_TYPE, 0x0, NULL,
  pool, forward_mp,
  &max_pooling2d_1_chain,
  NULL, &dense_1_dense_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(2, 2), 
  .pool_stride = AI_SHAPE_2D_INIT(2, 2), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  conv2d_1_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_1_conv2d_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_1_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  conv2d_1_layer, 0,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &conv2d_1_chain,
  NULL, &max_pooling2d_1_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  conv2d_1_conv2d_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &input_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_1_conv2d_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &conv2d_1_conv2d_weights, &conv2d_1_conv2d_bias, NULL),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  conv2d_1_conv2d_layer, 0,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &conv2d_1_conv2d_chain,
  NULL, &conv2d_1_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)


#if (AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 375448, 1, 1),
    375448, NULL, NULL),
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 92216, 1, 1),
    92216, NULL, NULL),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_MODEL_TEST1_IN_NUM, &input_0_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_MODEL_TEST1_OUT_NUM, &dense_3_output),
  &conv2d_1_conv2d_layer, 0, NULL)

#else

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 375448, 1, 1),
      375448, NULL, NULL)
  ),
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 92216, 1, 1),
      92216, NULL, NULL)
  ),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_MODEL_TEST1_IN_NUM, &input_0_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_MODEL_TEST1_OUT_NUM, &dense_3_output),
  &conv2d_1_conv2d_layer, 0, NULL)

#endif	/*(AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)*/


/******************************************************************************/
AI_DECLARE_STATIC
ai_bool model_test1_configure_activations(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_activations_map(g_model_test1_activations_map, 1, params)) {
    /* Updating activations (byte) offsets */
    
    input_0_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 0);
    input_0_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 0);
    
    conv2d_1_conv2d_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 1080);
    conv2d_1_conv2d_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 1080);
    
    conv2d_1_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 1080);
    conv2d_1_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 1080);
    
    max_pooling2d_1_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 1080);
    max_pooling2d_1_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 1080);
    
    dense_1_dense_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 0);
    dense_1_dense_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 0);
    
    dense_1_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 512);
    dense_1_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 512);
    
    dense_2_dense_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 0);
    dense_2_dense_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 0);
    
    dense_2_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 512);
    dense_2_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 512);
    
    dense_3_dense_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 0);
    dense_3_dense_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 0);
    
    dense_3_output_array.data = AI_PTR(g_model_test1_activations_map[0] + 24);
    dense_3_output_array.data_start = AI_PTR(g_model_test1_activations_map[0] + 24);
    
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_ACTIVATIONS);
  return false;
}



/******************************************************************************/
AI_DECLARE_STATIC
ai_bool model_test1_configure_weights(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_weights_map(g_model_test1_weights_map, 1, params)) {
    /* Updating weights (byte) offsets */
    
    conv2d_1_conv2d_bias_array.format |= AI_FMT_FLAG_CONST;
    conv2d_1_conv2d_bias_array.data = AI_PTR(g_model_test1_weights_map[0] + 0);
    conv2d_1_conv2d_bias_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 0);
    
    dense_1_dense_weights_array.format |= AI_FMT_FLAG_CONST;
    dense_1_dense_weights_array.data = AI_PTR(g_model_test1_weights_map[0] + 576);
    dense_1_dense_weights_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 512);
    
    dense_1_dense_bias_array.format |= AI_FMT_FLAG_CONST;
    dense_1_dense_bias_array.data = AI_PTR(g_model_test1_weights_map[0] + 361024);
    dense_1_dense_bias_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 361024);
    
    dense_2_dense_weights_array.format |= AI_FMT_FLAG_CONST;
    dense_2_dense_weights_array.data = AI_PTR(g_model_test1_weights_map[0] + 361600);
    dense_2_dense_weights_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 361536);
    
    dense_2_dense_bias_array.format |= AI_FMT_FLAG_CONST;
    dense_2_dense_bias_array.data = AI_PTR(g_model_test1_weights_map[0] + 369792);
    dense_2_dense_bias_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 369792);
    
    dense_3_dense_weights_array.format |= AI_FMT_FLAG_CONST;
    dense_3_dense_weights_array.data = AI_PTR(g_model_test1_weights_map[0] + 370304);
    dense_3_dense_weights_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 370304);
    
    dense_3_dense_bias_array.format |= AI_FMT_FLAG_CONST;
    dense_3_dense_bias_array.data = AI_PTR(g_model_test1_weights_map[0] + 373376);
    dense_3_dense_bias_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 373376);
    
    conv2d_1_conv2d_weights_array.format |= AI_FMT_FLAG_CONST;
    conv2d_1_conv2d_weights_array.data = AI_PTR(g_model_test1_weights_map[0] + 373400);
    conv2d_1_conv2d_weights_array.data_start = AI_PTR(g_model_test1_weights_map[0] + 373400);
    
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_WEIGHTS);
  return false;
}


/**  PUBLIC APIs SECTION  *****************************************************/


AI_DEPRECATED
AI_API_ENTRY
ai_bool ai_model_test1_get_info(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_MODEL_TEST1_MODEL_NAME,
      .model_signature   = AI_MODEL_TEST1_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 875232,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .params            = AI_STRUCT_INIT,
      .activations       = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0x0,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}


AI_API_ENTRY
ai_bool ai_model_test1_get_report(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_MODEL_TEST1_MODEL_NAME,
      .model_signature   = AI_MODEL_TEST1_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 875232,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .map_signature     = AI_MAGIC_SIGNATURE,
      .map_weights       = AI_STRUCT_INIT,
      .map_activations   = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0x0,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}

AI_API_ENTRY
ai_error ai_model_test1_get_error(ai_handle network)
{
  return ai_platform_network_get_error(network);
}

AI_API_ENTRY
ai_error ai_model_test1_create(
  ai_handle* network, const ai_buffer* network_config)
{
  return ai_platform_network_create(
    network, network_config, 
    &AI_NET_OBJ_INSTANCE,
    AI_TOOLS_API_VERSION_MAJOR, AI_TOOLS_API_VERSION_MINOR, AI_TOOLS_API_VERSION_MICRO);
}

AI_API_ENTRY
ai_error ai_model_test1_create_and_init(
  ai_handle* network, const ai_handle activations[], const ai_handle weights[])
{
    ai_error err;
    ai_network_params params;

    err = ai_model_test1_create(network, AI_MODEL_TEST1_DATA_CONFIG);
    if (err.type != AI_ERROR_NONE)
        return err;
    if (ai_model_test1_data_params_get(&params) != true) {
        err = ai_model_test1_get_error(*network);
        return err;
    }
#if defined(AI_MODEL_TEST1_DATA_ACTIVATIONS_COUNT)
    if (activations) {
        /* set the addresses of the activations buffers */
        for (int idx=0;idx<params.map_activations.size;idx++)
            AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);
    }
#endif
#if defined(AI_MODEL_TEST1_DATA_WEIGHTS_COUNT)
    if (weights) {
        /* set the addresses of the weight buffers */
        for (int idx=0;idx<params.map_weights.size;idx++)
            AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);
    }
#endif
    if (ai_model_test1_init(*network, &params) != true) {
        err = ai_model_test1_get_error(*network);
    }
    return err;
}

AI_API_ENTRY
ai_buffer* ai_model_test1_inputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    ((ai_network *)network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_inputs_get(network, n_buffer);
}

AI_API_ENTRY
ai_buffer* ai_model_test1_outputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    ((ai_network *)network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_outputs_get(network, n_buffer);
}

AI_API_ENTRY
ai_handle ai_model_test1_destroy(ai_handle network)
{
  return ai_platform_network_destroy(network);
}

AI_API_ENTRY
ai_bool ai_model_test1_init(
  ai_handle network, const ai_network_params* params)
{
  ai_network* net_ctx = ai_platform_network_init(network, params);
  if (!net_ctx) return false;

  ai_bool ok = true;
  ok &= model_test1_configure_weights(net_ctx, params);
  ok &= model_test1_configure_activations(net_ctx, params);

  ok &= ai_platform_network_post_init(network);

  return ok;
}


AI_API_ENTRY
ai_i32 ai_model_test1_run(
  ai_handle network, const ai_buffer* input, ai_buffer* output)
{
  return ai_platform_network_process(network, input, output);
}

AI_API_ENTRY
ai_i32 ai_model_test1_forward(ai_handle network, const ai_buffer* input)
{
  return ai_platform_network_process(network, input, NULL);
}



#undef AI_MODEL_TEST1_MODEL_SIGNATURE
#undef AI_NET_OBJ_INSTANCE
#undef AI_TOOLS_DATE_TIME
#undef AI_TOOLS_COMPILE_TIME

