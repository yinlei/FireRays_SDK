/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#ifndef FIRERAYS_TEST_H
#define FIRERAYS_TEST_H

/// This test suite is testing Firerays library functionality
///

#include "gtest/gtest.h"
#include "firerays.h"
#include "math/quaternion.h"

using namespace FireRays;

#include "tiny_obj_loader.h"

// Api creation fixture, prepares api_ for further tests
class Api : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        int nativeidx = -1;
        for (int idx=0; idx < IntersectionApi::GetDeviceCount(); ++idx)
        {
            DeviceInfo devinfo;
            IntersectionApi::GetDeviceInfo(idx, devinfo);
            
            if (devinfo.type == DeviceInfo::kGpu)
            {
                nativeidx = idx;
            }
        }

        ASSERT_NE(nativeidx, -1);

        api_ = IntersectionApi::Create(nativeidx);
    }

    virtual void TearDown()
    {
        IntersectionApi::Delete(api_);
    }

	void Wait()
	{
		e_->Wait();
		api_->DeleteEvent(e_);
	}

    IntersectionApi* api_;
	Event* e_;
};


// The test checks whether the api has been successfully created
TEST_F(Api, DeviceEnum)
{
    int numdevices = 0;
    ASSERT_NO_THROW(numdevices = IntersectionApi::GetDeviceCount());
    ASSERT_GT(numdevices, 0);
    
    for (int i=0; i<numdevices; ++i)
    {
        DeviceInfo devinfo;
        IntersectionApi::GetDeviceInfo(i, devinfo);
        
        ASSERT_NE(devinfo.name, nullptr);
        ASSERT_NE(devinfo.vendor, nullptr);
    }
}

// The test checks whether the api has been successfully created
TEST_F(Api, SingleDevice)
{
	ASSERT_TRUE(api_ != nullptr);
}

// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, Mesh)
{
    // Mesh vertices
    float vertices[] = {
        0.f,0.f,0.f,
        0.f,1.f,0.f,
        1.f,0.f,0.f
    };

    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };

    Shape* shape = nullptr;

    ASSERT_NO_THROW(shape = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(shape != nullptr);

    ASSERT_NO_THROW(api_->AttachShape(shape));
    ASSERT_NO_THROW(api_->DetachShape(shape));
    ASSERT_NO_THROW(api_->DeleteShape(shape));
}

// The test creates an empty scene
TEST_F(Api, EmptyScene)
{
	ASSERT_THROW(api_->Commit(), Exception);
}

// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, MeshStrided)
{
    struct Vertex
    {
        float position[3];
        float normal[3];
        float uv[2];
    };

    // Mesh vertices
    Vertex vertices[] = {
        { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f },
        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
    };

    // Indices
    int indices[] = { 0, 1, 2, 0, 0, 1, 2, 0};

    Shape* shape = nullptr;

    ASSERT_NO_THROW(shape = api_->CreateMesh((float*)vertices, 6, sizeof(Vertex), indices, 4*sizeof(int), nullptr, 2));

    ASSERT_TRUE(shape != nullptr);

    ASSERT_NO_THROW(api_->AttachShape(shape));
    ASSERT_NO_THROW(api_->DetachShape(shape));
    ASSERT_NO_THROW(api_->DeleteShape(shape));
}



//The test creates a single triangle mesh and then tries to create an instance of the mesh
TEST_F(Api, Instance)
{
    // Mesh vertices
    float vertices[] = {
        0.f,0.f,0.f,
        0.f,1.f,0.f,
        1.f,0.f,0.f
    };
    
    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };
    
    Shape* shape = nullptr;
    
    ASSERT_NO_THROW(shape = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices,0, numfaceverts, 1));
    
    ASSERT_TRUE(shape != nullptr);
    
    ASSERT_NO_THROW(api_->AttachShape(shape));
    ASSERT_NO_THROW(api_->DetachShape(shape));
    
    Shape* instance = nullptr;
    
    ASSERT_NO_THROW(instance = api_->CreateInstance(shape));
    
    ASSERT_TRUE(instance != nullptr);
    
    ASSERT_NO_THROW(api_->DeleteShape(shape));
}

// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, Intersection_1Ray)
{
    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };

    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };

    Shape* mesh = nullptr;

    // Create mesh
    ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(mesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(mesh));

    // Prepare the ray
    ray r(float3(0.f, 0.f, -10.f), float3(0.f, 0.f, 1.f), 10000.f);

    // Intersection and hit data
    Intersection isect;

    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);
    
    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));

    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();

    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_)); 
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, mesh->GetId());

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(mesh));
    ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}


// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, Intersection_1Ray_Masked)
{
	// Mesh vertices
	float vertices[] = {
		-1.f,-1.f,0.f,
		1.f,-1.f,0.f,
		0.f,1.f,0.f,

	};

	// Indices
	int indices[] = { 0, 1, 2 };
	// Number of vertices for the face
	int numfaceverts[] = { 3 };

	Shape* mesh = nullptr;

	api_->SetOption("acc.type", "bvh");
	//api_->SetOption("bvh.force2level", 1.f);

	// Create mesh
	ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3 * sizeof(float), indices, 0, numfaceverts, 1));

	// Set mask 
	ASSERT_NO_THROW(mesh->SetMask(0xFFFFFFFF));

	ASSERT_TRUE(mesh != nullptr);

	// Attach the mesh to the scene
	ASSERT_NO_THROW(api_->AttachShape(mesh));

	// Prepare the ray
	ray r(float3(0.f, 0.f, -10.f), float3(0.f, 0.f, 1.f), 10000.f);
	r.SetMask(0xFFFFFFFF);

	// Intersection and hit data
	Intersection isect;

    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);
    auto isect_flag_buffer = api_->CreateBuffer(sizeof(int), nullptr);

	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());
    
	// Intersect
	ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
	// Check results
	ASSERT_EQ(isect.shapeid, mesh->GetId());

	mesh->SetMask(0x0);

	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());
	
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
	// Check results
	ASSERT_EQ(isect.shapeid, kNullId);

	mesh->SetMask(0xFF000000);
    
	int result = kNullId;
	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());
    // Intersect
    ASSERT_NO_THROW(api_->QueryOcclusion(ray_buffer, 1, isect_flag_buffer, nullptr, nullptr ));
    
    int* isect_flag = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_flag_buffer, kMapRead, 0, sizeof(int), (void**)&isect_flag, &e_));
	Wait();
    result = *isect_flag;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_flag_buffer, isect_flag, &e_));
	Wait();
    
	// Check results
	ASSERT_GT(result, 0);

	mesh->SetMask(0xFF000000);
    
	r.SetMask(0x000000FF);
    
    ray* rr = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(ray_buffer, kMapWrite, 0, sizeof(ray), (void**)&rr, &e_));
	Wait();
    *rr = r;
    ASSERT_NO_THROW(api_->UnmapBuffer(ray_buffer, rr, &e_));
	Wait();

	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());
    // Intersect
    ASSERT_NO_THROW(api_->QueryOcclusion(ray_buffer, 1, isect_flag_buffer, nullptr, nullptr ));
    
    isect_flag = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_flag_buffer, kMapRead, 0, sizeof(int), (void**)&isect_flag, &e_));
	Wait();
    result = *isect_flag;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_flag_buffer, isect_flag, &e_));
	Wait();
	// Check results
	ASSERT_EQ(result, kNullId);


	// Bail out
	ASSERT_NO_THROW(api_->DetachShape(mesh));
	ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_flag_buffer));

}

// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, Intersection_1Ray_Active)
{
	// Mesh vertices
	float vertices[] = {
		-1.f,-1.f,0.f,
		1.f,-1.f,0.f,
		0.f,1.f,0.f,

	};

	// Indices
	int indices[] = { 0, 1, 2 };
	// Number of vertices for the face
	int numfaceverts[] = { 3 };

	Shape* mesh = nullptr;

	// Create mesh
	ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3 * sizeof(float), indices, 0, numfaceverts, 1));

	ASSERT_TRUE(mesh != nullptr);

	// Attach the mesh to the scene
	ASSERT_NO_THROW(api_->AttachShape(mesh));

	// Prepare the ray
	ray r(float3(0.f, 0.f, -10.f), float3(0.f, 0.f, 1.f), 10000.f);

	// Intersection and hit data
	Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);

	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
	// Check results
	ASSERT_EQ(isect.shapeid, mesh->GetId());

	isect.primid = kNullId;
	isect.shapeid = kNullId;

	r.SetActive(false);
    
    ray* rr = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(ray_buffer, kMapWrite, 0, sizeof(ray), (void**)&rr, &e_));
	Wait();
    *rr = r;
    ASSERT_NO_THROW(api_->UnmapBuffer(ray_buffer, rr, &e_));
	Wait();
    
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapWrite, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    *tmp = isect;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();

    
	// Check results
	ASSERT_EQ(isect.shapeid, kNullId);


	// Bail out
	ASSERT_NO_THROW(api_->DetachShape(mesh));
	ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}

// The test creates a single triangle mesh and tests attach/detach functionality
TEST_F(Api, Intersection_3Rays)
{
    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };

    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };

    Shape* mesh = nullptr;

    // 
    ASSERT_NO_THROW(api_->SetOption("acc.type", "grid"));

    // Create mesh
    ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(mesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(mesh));

    // Rays
    ray rays[3];

    // Prepare the ray
    rays[0].o = float4(0.f,0.f,-10.f, 1000.f);
    rays[0].d = float3(0.f,0.f,1.f);

    rays[1].o = float4(0.f,0.5f,-10.f, 1000.f);
    rays[1].d = float3(0.f,0.f,1.f);

    rays[2].o = float4(0.5f,0.f,-10.f, 1000.f);
    rays[2].d = float3(0.f,0.f,1.f);

    // Intersection and hit data
    Intersection isect[3];
    
    auto ray_buffer = api_->CreateBuffer(3*sizeof(ray), rays);
    auto isect_buffer = api_->CreateBuffer(3*sizeof(Intersection), nullptr);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 3, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, 3*sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect[0] = tmp[0];
    isect[1] = tmp[1];
    isect[2] = tmp[2];
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    for (int i=0; i<3; ++i)
    {
        ASSERT_EQ(isect[i].shapeid, mesh->GetId());
    }

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(mesh));
    ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}


// Test is checking if mesh transform is working as expected
TEST_F(Api, Intersection_1Ray_Transformed)
{
    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };

    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };

    Shape* mesh = nullptr;

    // Create mesh
    ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(mesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(mesh));

    // Prepare the ray
    ray r;
    r.o = float4(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Intersection and hit data
    Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, mesh->GetId());

    matrix m = translation(float3(0,2,0));
    matrix minv = inverse(m);
    // Move the mesh
    ASSERT_NO_THROW(mesh->SetTransform(m, minv));
    // Reset ray

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, -1);

    // Set transform to identity
    m = matrix();
    ASSERT_NO_THROW(mesh->SetTransform(m, m));

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, mesh->GetId());

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(mesh));
    ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}

// The test checks intersection after geometry addition
TEST_F(Api, Intersection_1Ray_DynamicGeo)
{
    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };
    
    float vertices1[] = {
        -1.f,-1.f,-1.f,
        1.f,-1.f,-1.f,
        0.f,1.f,-1.f,
        
    };

    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };

    Shape* closemesh = nullptr;
    Shape* farmesh = nullptr;

    // Create two meshes
    ASSERT_NO_THROW(farmesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));
    ASSERT_NO_THROW(closemesh = api_->CreateMesh(vertices1, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(farmesh != nullptr);
    ASSERT_TRUE(closemesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(farmesh));

    // Prepare the ray
    ray r;
    r.o = float4(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);


    // Intersection and hit data
    Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, farmesh->GetId());

    // Attach closer mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(closemesh));

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, closemesh->GetId());

    // Attach closer mesh to the scene
    ASSERT_NO_THROW(api_->DetachShape(closemesh));

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, farmesh->GetId());

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(farmesh));
    ASSERT_NO_THROW(api_->DeleteShape(farmesh));
    ASSERT_NO_THROW(api_->DeleteShape(closemesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}

TEST_F(Api, CornellBoxLoad)
{
    using namespace tinyobj;
    std::vector<shape_t> shapes;
    std::vector<material_t> materials;
    std::vector<Shape*> apishapes;

    // Load obj file 
    std::string res = LoadObj(shapes, materials, "../Resources/CornellBox/orig.objm");

    ASSERT_NO_THROW(api_->SetOption("acc.type", "grid"));

    // Create meshes within IntersectionApi
    for  (int i=0; i<(int)shapes.size(); ++i)
    {
        Shape* shape = nullptr;
        ASSERT_NO_THROW(shape = api_->CreateMesh(&shapes[i].mesh.positions[0], (int)shapes[i].mesh.positions.size() / 3, 3*sizeof(float),
            &shapes[i].mesh.indices[0], 0, nullptr, (int)shapes[i].mesh.indices.size() / 3));

        ASSERT_NO_THROW(api_->AttachShape(shape));
        apishapes.push_back(shape);
    }

    // Commit update
    ASSERT_NO_THROW(api_->Commit());

    // Delete meshes
    for (int i=0; i<(int)apishapes.size(); ++i)
    {
        ASSERT_NO_THROW(api_->DeleteShape(apishapes[i]));
    }
}

TEST_F(Api, CornellBox_1Ray)
{
    using namespace tinyobj;
    std::vector<shape_t> shapes;
    std::vector<material_t> materials;
    std::vector<Shape*> apishapes;

    // Load obj file 
    std::string res = LoadObj(shapes, materials, "../Resources/CornellBox/orig.objm");

    //ASSERT_NO_THROW(api_->SetOption("acc.type", "grid"));

    // Create meshes within IntersectionApi
    for (int i = 0; i<(int)shapes.size(); ++i)
    {
        Shape* shape = nullptr;
        ASSERT_NO_THROW(shape = api_->CreateMesh(&shapes[i].mesh.positions[0], (int)shapes[i].mesh.positions.size() / 3, 3 * sizeof(float),
            &shapes[i].mesh.indices[0], 0, nullptr, (int)shapes[i].mesh.indices.size() / 3));

        ASSERT_NO_THROW(api_->AttachShape(shape));
        apishapes.push_back(shape);
    }

    // Prepare the ray
    ray r;
    r.o = float4(0.f, 0.5f, -10.f, 1000.f);
    r.d = float3(0.f, 0.f, 1.f);


    // Intersection and hit data
    Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();


    // Delete meshes
    for (int i = 0; i<(int)apishapes.size(); ++i)
    {
        ASSERT_NO_THROW(api_->DeleteShape(apishapes[i]));
    }

	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}


// Test is checking if mesh transform is working as expected
TEST_F(Api, Intersection_1Ray_TransformedInstance)
{
    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };
    
    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };
    
    Shape* mesh = nullptr;

    // Create mesh
    ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(mesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(mesh));

    // Prepare the ray
    ray r;
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Intersection and hit data
    Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);
    
    // Create instance of a triangle
    Shape* instance = nullptr;
    ASSERT_NO_THROW(instance = api_->CreateInstance(mesh));
    
    matrix m = translation(float3(0,0,-2));
    matrix minv = inverse(m);
    ASSERT_NO_THROW(instance->SetTransform(m, minv));
    
    ASSERT_NO_THROW(api_->AttachShape(instance));
    
    // Prepare the ray
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, instance->GetId());
    ASSERT_LE(std::fabs(isect.uvwt.w - 8.f), 0.01f);
    
    //
    m = translation(float3(0,0,2));
    minv = inverse(m);
    ASSERT_NO_THROW(instance->SetTransform(m, minv));

    // Prepare the ray
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();

    // Check results
    ASSERT_EQ(isect.shapeid, mesh->GetId());
    ASSERT_LE(std::fabs(isect.uvwt.w - 10.f), 0.01f);

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(instance));
    ASSERT_NO_THROW(api_->DeleteShape(instance));
    ASSERT_NO_THROW(api_->DetachShape(mesh));
    ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}

TEST_F(Api, Intersection_1Ray_TransformedInstanceFlat)
{

	// Set flattening
	api_->SetOption("bvh.forceflat", 1.f);

    // Mesh vertices
    float vertices[] = {
        -1.f,-1.f,0.f,
        1.f,-1.f,0.f,
        0.f,1.f,0.f,
        
    };
    
    // Indices
    int indices[] = {0, 1, 2};
    // Number of vertices for the face
    int numfaceverts[] = { 3 };
    
    Shape* mesh = nullptr;

    // Create mesh
    ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3*sizeof(float), indices, 0, numfaceverts, 1));

    ASSERT_TRUE(mesh != nullptr);

    // Attach the mesh to the scene
    ASSERT_NO_THROW(api_->AttachShape(mesh));

    // Prepare the ray
    ray r;
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Intersection and hit data
    Intersection isect;
    
    auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
    auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);
    
    // Create instance of a triangle
    Shape* instance = nullptr;
    ASSERT_NO_THROW(instance = api_->CreateInstance(mesh));
    
    matrix m = translation(float3(0,0,-2));
    matrix minv = inverse(m);
    ASSERT_NO_THROW(instance->SetTransform(m, minv));
    
    ASSERT_NO_THROW(api_->AttachShape(instance));
    
    // Prepare the ray
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    Intersection* tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();
    
    // Check results
    ASSERT_EQ(isect.shapeid, instance->GetId());
    ASSERT_LE(std::fabs(isect.uvwt.w - 8.f), 0.01f);
    
    //
    m = translation(float3(0,0,2));
    minv = inverse(m);
    ASSERT_NO_THROW(instance->SetTransform(m, minv));

    // Prepare the ray
    r.o = float3(0.f,0.f,-10.f, 1000.f);
    r.d = float3(0.f,0.f,1.f);

    // Commit geometry update
    ASSERT_NO_THROW(api_->Commit());
    
    // Intersect
    ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr ));
    
    tmp = nullptr;
    ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
    isect = *tmp;
    ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();

    // Check results
    ASSERT_EQ(isect.shapeid, mesh->GetId());
    ASSERT_LE(std::fabs(isect.uvwt.w - 10.f), 0.01f);

    // Bail out
    ASSERT_NO_THROW(api_->DetachShape(instance));
    ASSERT_NO_THROW(api_->DeleteShape(instance));
    ASSERT_NO_THROW(api_->DetachShape(mesh));
    ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}
// Test is checking if mesh transform is working as expected
// DK: #22 repro case : Commit throws if base shape has not been attached
TEST_F(Api, Intersection_1Ray_InstanceNoShape)
{
	// Mesh vertices
	float vertices[] = {
		-1.f,-1.f,0.f,
		1.f,-1.f,0.f,
		0.f,1.f,0.f,

	};

	// Indices
	int indices[] = { 0, 1, 2 };
	// Number of vertices for the face
	int numfaceverts[] = { 3 };

	Shape* mesh = nullptr;

	// Create mesh
	ASSERT_NO_THROW(mesh = api_->CreateMesh(vertices, 3, 3 * sizeof(float), indices, 0, numfaceverts, 1));

	ASSERT_TRUE(mesh != nullptr);

	// Attach the mesh to the scene
	//ASSERT_NO_THROW(api_->AttachShape(mesh));

	// Prepare the ray
	ray r;
	r.o = float3(0.f, 0.f, -10.f, 1000.f);
	r.d = float3(0.f, 0.f, 1.f);

	// Intersection and hit data
	Intersection isect;

	auto ray_buffer = api_->CreateBuffer(sizeof(ray), &r);
	auto isect_buffer = api_->CreateBuffer(sizeof(Intersection), nullptr);

	// Create instance of a triangle
	Shape* instance = nullptr;
	ASSERT_NO_THROW(instance = api_->CreateInstance(mesh));

	matrix m = translation(float3(0, 0, 2));
	matrix minv = inverse(m);
	ASSERT_NO_THROW(instance->SetTransform(m, minv));

	ASSERT_NO_THROW(api_->AttachShape(instance));

	// Prepare the ray
	r.o = float3(0.f, 0.f, -10.f, 1000.f);
	r.d = float3(0.f, 0.f, 1.f);

	// Commit geometry update
	ASSERT_NO_THROW(api_->Commit());

	// Intersect
	ASSERT_NO_THROW(api_->QueryIntersection(ray_buffer, 1, isect_buffer, nullptr, nullptr));

	Intersection* tmp = nullptr;
	ASSERT_NO_THROW(api_->MapBuffer(isect_buffer, kMapRead, 0, sizeof(Intersection), (void**)&tmp, &e_));
	Wait();
	isect = *tmp;
	ASSERT_NO_THROW(api_->UnmapBuffer(isect_buffer, tmp, &e_));
	Wait();

	// Check results
	ASSERT_EQ(isect.shapeid, instance->GetId());
	ASSERT_LE(std::fabs(isect.uvwt.w - 12.f), 0.01f);

	// Bail out
	ASSERT_NO_THROW(api_->DetachShape(instance));
	ASSERT_NO_THROW(api_->DeleteShape(instance));
	ASSERT_NO_THROW(api_->DetachShape(mesh));
	ASSERT_NO_THROW(api_->DeleteShape(mesh));
	ASSERT_NO_THROW(api_->DeleteBuffer(ray_buffer));
	ASSERT_NO_THROW(api_->DeleteBuffer(isect_buffer));
}


#endif // FIRERAYS_TEST_H
